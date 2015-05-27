Imports System.IO

Public Class HEX_FirmwareProcessor
    Const ESP_BLOCK_SIZE As Integer = 256
    Const MCU_FirmwareInstructionCount = &H13C00

    Shared Function Build_MCU_FirmwareBIN(inp As Stream) As Stream
        Dim programArea((MCU_FirmwareInstructionCount * 4) - 1) As Byte 'Each Instruction is 4 bytes long
        Dim x As Integer

        For x = 0 To programArea.Length - 1
            programArea(x) = &HFF
        Next

        Dim SegmentAddressOffset As UInt32 = 0
        Dim LinearAddressOffset As UInt32 = 0
        Using hexfile As New StreamReader(inp)
            While 1
                Dim line = hexfile.ReadLine
                If line.Substring(0, 1) <> ":" Then Throw New InvalidOperationException("Corrupted Hex File")
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
        Dim FirmwareBinLength As UInt32 = (MCU_FirmwareInstructionCount * 3) 'We only need to store 24-bits per instruction the last byte is always blank
        ms.Write(BitConverter.GetBytes(FirmwareBinLength), 0, 4)
        While (x < programArea.Length)
            ms.Write(programArea, x, 3)
            x += 4
        End While
        ms.Position = 0
        Return ms
    End Function

    Shared Function Build_ESP_FirmwareBIN(inp As Stream, Offset As UInt32) As Stream
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

    Shared Function GenerateMasterBIN(folder As String) As Stream
        Dim Master As New MemoryStream

        Try
            Using MCU_FirmwareBin = Build_MCU_FirmwareBIN(New FileStream(folder & "\mcu_firmware.hex", FileMode.Open, FileAccess.Read))
                MCU_FirmwareBin.Position = 0
                MCU_FirmwareBin.CopyTo(Master)
            End Using
        Catch ex As Exception
            Console.WriteLine("Error: Unable to Process the mcu_firmware.hex file...Ex=" & ex.ToString)
            Environment.Exit(-1)
        End Try

        For Each fname In Directory.GetFiles(folder, "*.bin")
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
        Master.Position = 0
        Return Master
    End Function
End Class
