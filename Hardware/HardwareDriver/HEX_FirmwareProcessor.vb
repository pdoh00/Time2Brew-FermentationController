Imports System.IO

Public Class HEX_FirmwareProcessor
    Shared Function GetBinary(inp As IO.Stream, FirmwareLength_wordCount As Integer) As IO.Stream
        Dim programArea((FirmwareLength_wordCount * 2) - 1) As Byte
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
        While (x < programArea.Length)
            ms.Write(programArea, x, 3)
            x += 4
        End While
        ms.Position = 0

        Return ms

    End Function
End Class
