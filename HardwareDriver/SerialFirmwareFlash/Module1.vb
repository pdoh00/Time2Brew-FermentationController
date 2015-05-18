Imports System.IO

Module Module1
    Const ACK As Byte = &HF1
    Const NACK As Byte = &HF2
    Const ACK_WAIT As Byte = &HF3
    Const RQ_CONFIRM As Byte = &HF4
    Const CMD_ECHO As Byte = &HA0
    Const CMD_ERASE_BACKUP As Byte = &H2
    Const CMD_ERASE_PRIMARY As Byte = &H3
    Const CMD_CONFIRM As Byte = &H4
    Const CMD_UPLOAD_BACKUP As Byte = &H5
    Const CMD_UPLOAD_PRIMARY As Byte = &H6
    Const CMD_FLASH_BACKUP As Byte = &H7
    Const CMD_FLASH_PRIMARY As Byte = &H8
    Const CMD_SEND_PRIMARY As Byte = &HA
    Const CMD_SEND_BACKUP As Byte = &H9
    Const CMD_WIPE As Byte = &HB


    Const FirmwareLength = &H27800

    Sub Main()
        Dim args = Environment.GetCommandLineArgs()

        If args.Count <> 3 Then
            Console.WriteLine("Args are not good!")
            Return
        End If

        If File.Exists(args(1)) = False Then
            Console.WriteLine("Unable to find the HEX file...")
            Return
        End If

        Dim programArea((FirmwareLength * 2) - 1) As Byte
        Dim x As Integer

        For x = 0 To programArea.Length - 1
            programArea(x) = &HFF
        Next

        Dim SegmentAddressOffset As UInt32 = 0
        Dim LinearAddressOffset As UInt32 = 0

        Using hexfile As New StreamReader(args(1))
            While 1
                Dim line = hexfile.ReadLine
                If line.Substring(0, 1) <> ":" Then Throw New ApplicationException("Corrupted Hex File")
                Dim byteCount = CUInt("&H" & line.Substring(1, 2))
                Dim lowerAddress = CUInt("&H" & line.Substring(3, 4))
                Dim recordType = CByte("&H" & line.Substring(7, 2))

                Select Case recordType
                    Case 0  'Data Record
                        Dim BaseAddress As UInt32 = (LinearAddressOffset << 16) + (SegmentAddressOffset << 4) + lowerAddress
                        For bb As Integer = 0 To byteCount - 1
                            If (BaseAddress) < programArea.Length Then
                                programArea(BaseAddress) = CByte("&H" & line.Substring(9 + (bb * 2), 2))
                            End If
                            BaseAddress += 1
                        Next
                    Case 1  'End of File
                        Exit While
                    Case 2  'Extended Segment Address Record
                        SegmentAddressOffset = CUShort("&H" & line.Substring(9, 4))
                    Case 3  'Start Segement Address Record
                    Case 4  'Extended Linear Address Record
                        LinearAddressOffset = CUShort("&H" & line.Substring(9, 4))
                    Case 5  'Start Linear Address Record 
                End Select

            End While
        End Using

        Dim ms As New MemoryStream()

        x = 0
        ms.WriteByte(&HAA)
        While (x < programArea.Length)
            ms.Write(programArea, x, 3)
            x += 4
        End While
        ms.Position = 0

        Dim com As New IO.Ports.SerialPort(args(2), 460800, Ports.Parity.None, 8, Ports.StopBits.One)
        Dim resp As Integer
        com.Open()
        com.ReadTimeout = 100
        com.WriteTimeout = 100

        Console.WriteLine("Connecting...")
        While (1)
            com.Write(New Byte() {CMD_ECHO}, 0, 1)
            If com.BytesToRead > 0 Then
                resp = com.ReadByte
                If resp = ACK Then Exit While
            End If
            While com.BytesToWrite
            End While
        End While

        Console.WriteLine("Connected OK")

        Console.WriteLine("WIPE CHIP...")
        com.ReadExisting()
        com.Write(New Byte() {CMD_WIPE}, 0, 1)
        resp = com.ReadByte
        If resp <> RQ_CONFIRM Then
            Console.WriteLine("Oops: Rx=" & resp)
            Console.ReadLine()
        End If
        Console.WriteLine("Request For Confirmation...")
        com.Write(New Byte() {CMD_CONFIRM}, 0, 1)
        resp = com.ReadByte
        If resp <> ACK_WAIT Then
            Console.WriteLine("Oops: Rx=" & resp)
            Console.ReadLine()
        End If
        Console.WriteLine("ACK Wait recieved...polling for ack")
        com.ReadTimeout = 90000

        resp = com.ReadByte
        If resp <> ACK Then
            Console.WriteLine("ACK not received...instead Rx=" & resp)
            Console.ReadLine()
        End If
        Console.WriteLine("ACK Recieved!")

        Dim tick As Integer = 0


        ms.Position = 0
        While ms.Position < ms.Length
            Using tx As New MemoryStream
                Dim offset As UInt32 = ms.Position
                tx.Write(BitConverter.GetBytes(offset), 0, 4)
                Dim buff(192) As Byte
                ms.Read(buff, 0, 192)
                tx.Write(buff, 0, 192)
                tx.Position = 0
                Dim chkSum As UInt16 = FletcherChecksum.Fletcher16(tx.ToArray, 0, tx.Length)
                tx.Position = tx.Length
                tx.Write(BitConverter.GetBytes(chkSum), 0, 2)
                tx.Position = 0
                While (1)
                    com.Write(New Byte() {CMD_UPLOAD_BACKUP}, 0, 1)
                    resp = com.ReadByte
                    If (resp = ACK) Then
                        tick += 1
                        If (tick > 10) Then
                            tick = 0
                            Console.WriteLine("Sending Data. MS.Position=" & ms.Position)
                        End If
                        com.Write(tx.ToArray, 0, tx.Length)
                        resp = com.ReadByte
                        If (resp <> ACK) Then
                            Console.WriteLine("FAIL: ACK not Received: Rx=" & resp)
                            Console.ReadLine()
                            Environment.Exit(-1)
                        Else
                            Exit While
                        End If
                    End If
                End While

            End Using
        End While
        Console.WriteLine("Upload Complete!!!")

        Console.WriteLine("Flashing Firware")
        com.Write(New Byte() {CMD_FLASH_BACKUP}, 0, 1)
        resp = com.ReadByte
        If resp <> RQ_CONFIRM Then
            Console.WriteLine("Oops: Rx=" & resp)
            Console.ReadLine()
        End If
        Console.WriteLine("Request For Confirmation...")
        com.Write(New Byte() {CMD_CONFIRM}, 0, 1)

        While (1)
            If com.BytesToRead > 0 Then
                Console.Write(com.ReadExisting)
            End If
        End While


        Console.ReadLine()


    End Sub

End Module
