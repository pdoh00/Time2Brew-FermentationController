Imports System.IO
Imports System.IO.Ports

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
    Const CMD_WIPE_CHIP As Byte = &HB
    Const CMD_SEND_BACKUP As Byte = &H9
    Const CMD_SET_SECURITY As Byte = &HE

    Const ESP_BLOCK_SIZE As Integer = 256
    Const FirmwareInstructionCount = &H13C00

    Function Build_MCU_FirmwareBIN(inp As Stream) As Stream
        Dim programArea((FirmwareInstructionCount * 4) - 1) As Byte 'Each Instruction is 4 bytes long
        Dim x As Integer

        For x = 0 To programArea.Length - 1
            programArea(x) = &HFF
        Next

        Dim SegmentAddressOffset As UInt32 = 0
        Dim LinearAddressOffset As UInt32 = 0
        Using hexfile As New StreamReader(inp)
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
        Dim FirmwareBinLength As UInt32 = (FirmwareInstructionCount * 3) 'We only need to store 24-bits per instruction the last byte is always blank
        ms.Write(BitConverter.GetBytes(FirmwareBinLength), 0, 4)
        While (x < programArea.Length)
            ms.Write(programArea, x, 3)
            x += 4
        End While
        ms.Position = 0
        Return ms
    End Function

    Function Build_ESP_FirmwareBIN(inp As Stream, Offset As UInt32) As Stream
        Dim num_blocks As UInt32 = Math.Ceiling(inp.Length / ESP_BLOCK_SIZE)
        Dim BIN_Length = num_blocks * ESP_BLOCK_SIZE

        Dim rawBuffer(BIN_Length - 1) As Byte
        For idx As Integer = 0 To BIN_Length - 1
            rawBuffer(idx) = &HE0
        Next

        Dim ms As New MemoryStream
        ms.Write(BitConverter.GetBytes(Offset), 0, 4)
        ms.Write(BitConverter.GetBytes(BIN_Length), 0, 4)
        inp.Read(rawBuffer, 0, rawBuffer.Length)
        ms.Write(rawBuffer, 0, rawBuffer.Length)
        Return ms
    End Function

    Function Connect(com As SerialPort) As Boolean
        Dim resp As Integer
        Console.Write("Connecting...")
        While (1)
            com.Write(New Byte() {CMD_ECHO}, 0, 1)
            If com.BytesToRead > 0 Then
                resp = com.ReadByte
                If resp = ACK Then Exit While
            End If
            While com.BytesToWrite
            End While
        End While
        Console.WriteLine("OK")
        Return True
    End Function

    Function WipeChip(com As SerialPort) As Boolean
        Dim resp As Integer
        Console.WriteLine("Wipe Chip...")
        com.ReadExisting()
        com.Write(New Byte() {CMD_WIPE_CHIP}, 0, 1)
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
        com.ReadTimeout = 90000 '90 Second timeout

        Try
            resp = com.ReadByte
        Catch ex As Exception
            Console.WriteLine("Error: Ex=" & ex.ToString)
            Return False
        End Try

        If resp <> ACK Then
            Console.WriteLine("ACK not received...instead Rx=" & resp)
            Console.ReadLine()
        End If
        Console.WriteLine("Wipe Chip Complete")
        Return True
    End Function

    Function EraseBackup(com As SerialPort) As Boolean
        Dim resp As Integer
        Console.WriteLine("Erase Backup...")
        com.ReadExisting()
        com.Write(New Byte() {CMD_ERASE_BACKUP}, 0, 1)
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
        com.ReadTimeout = 10000 '10 second timeout

        Try
            resp = com.ReadByte
        Catch ex As Exception
            Console.WriteLine("Error: Ex=" & ex.ToString)
            Return False
        End Try

        If resp <> ACK Then
            Console.WriteLine("ACK not received...instead Rx=" & resp)
            Console.ReadLine()
        End If
        Console.WriteLine("Erase Backup Complete")
        Return True
    End Function

    Function UploadBackupFirmware(com As SerialPort, MS As Stream) As Boolean
        Dim resp As Integer
        Dim tick As Integer

        MS.Position = 0
        While MS.Position < MS.Length
            Using tx As New MemoryStream
                Dim offset As UInt32 = MS.Position
                tx.Write(BitConverter.GetBytes(offset), 0, 4)
                Dim buff(255) As Byte
                MS.Read(buff, 0, 256)
                tx.Write(buff, 0, 256)
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
                            Console.WriteLine("Sending Data. MS.Position=" & MS.Position)
                        End If
                        com.Write(tx.ToArray, 0, tx.Length)
                        resp = com.ReadByte
                        If (resp <> ACK) Then
                            Console.WriteLine("FAIL: ACK not Received: Rx=" & resp)
                            Return False
                        Else
                            Exit While
                        End If
                    End If
                End While

            End Using
        End While
        Console.WriteLine("Upload Complete!!!")
        Return True
    End Function

    Function FlashBackupFirmware(com As SerialPort) As Boolean
        Dim resp As Integer

        Console.WriteLine("Flashing Backup Firware")
        com.Write(New Byte() {CMD_FLASH_BACKUP}, 0, 1)
        resp = com.ReadByte
        If resp <> RQ_CONFIRM Then
            Console.WriteLine("Oops: Rx=" & resp)
            Return False
        End If
        Console.WriteLine("Request For Confirmation...")
        com.Write(New Byte() {CMD_CONFIRM}, 0, 1)
        Console.WriteLine("Complete")
        Return True
    End Function

    Sub Main()
        Dim args = Environment.GetCommandLineArgs()

        If args.Count <> 4 Then
            Console.WriteLine("Usage: SerialFirmwareFlash.exe Firmware_Set_Directory COM_PORT_NAME  BAUD_RATE")
            Environment.Exit(-1)
        End If

        If Directory.Exists(args(1)) = False Then
            Console.WriteLine("Unable to find the Firwmare Set Directory...")
            Return
        End If

        Dim Master As New MemoryStream

        Try
            Using MCU_FirmwareBin = Build_MCU_FirmwareBIN(New FileStream(args(1) & "\mcu_firmware.hex", FileMode.Open, FileAccess.Read))
                MCU_FirmwareBin.Position = 0
                MCU_FirmwareBin.CopyTo(Master)
            End Using
        Catch ex As Exception
            Console.WriteLine("Error: Unable to Process the mcu_firmware.hex file...Ex=" & ex.ToString)
            Environment.Exit(-1)
        End Try

        For Each fname In Directory.GetFiles(args(1), "*.bin")
            Try
                Dim offset As UInt32 = Path.GetFileNameWithoutExtension(fname).Replace("0x", "&h")
                Using fs As New FileStream(fname, FileMode.Open, FileAccess.Read)
                    Using bin = Build_ESP_FirmwareBIN(fs, offset)
                        bin.Position = 0
                        bin.CopyTo(Master)
                    End Using
                End Using
            Catch ex As Exception
                Console.WriteLine("Error: Unable to Process the esp firmware bin file: " & fname & " : Ex=" & ex.ToString)
                Environment.Exit(-1)
            End Try

        Next

        Dim com As New IO.Ports.SerialPort(args(2), args(3), Ports.Parity.None, 8, Ports.StopBits.One)
        Try
            com.Open()
        Catch ex As Exception
            Console.WriteLine("Unable to Open COM Port: " & ex.ToString)
            Environment.Exit(-5)
        End Try

        com.ReadTimeout = 100
        com.WriteTimeout = 100

        If (Connect(com) = False) Then
            Environment.Exit(-1)
        End If

        If WipeChip(com) = False Then
            Environment.Exit(-2)
        End If

        If UploadBackupFirmware(com, Master) = False Then
            Environment.Exit(-3)
        End If

        If FlashBackupFirmware(com) = False Then
            Environment.Exit(-4)
        End If

        While (1)
            Dim token = com.ReadExisting()
            Console.Write(token)
        End While

    End Sub

End Module
