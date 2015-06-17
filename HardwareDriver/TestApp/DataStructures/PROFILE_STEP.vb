Imports System.IO

Public Structure PROFILE_STEP
    Public StartTemperature_C As Single
    Public EndTemperature_C As Single

    Public Duration_seconds As UInt32
    Public Function Serialize() As Byte()
        Using ms As New MemoryStream()
            ms.Write(BitConverter.GetBytes(StartTemperature_C), 0, 4)
            ms.Write(BitConverter.GetBytes(EndTemperature_C), 0, 4)
            ms.Write(BitConverter.GetBytes(Duration_seconds), 0, 4)
            Return ms.ToArray()
        End Using
    End Function

    Public Sub New(sourceData As Byte(), sourceOffset As Integer)
        Dim offset As Integer = sourceOffset
        StartTemperature_C = CSng(BitConverter.ToSingle(sourceData, offset))
        offset += 4
        EndTemperature_C = CSng(BitConverter.ToSingle(sourceData, offset))
        offset += 4
        Duration_seconds = BitConverter.ToUInt32(sourceData, offset)
        offset += 4
    End Sub

    Public Sub New(StartTemp As Double, EndTemp As Double, Duration As Integer)
        Me.StartTemperature_C = StartTemp
        Me.EndTemperature_C = EndTemp
        Me.Duration_seconds = Duration
    End Sub

    Public Overrides Function ToString() As String
        Return StartTemperature_C.ToString & "C to " & EndTemperature_C.ToString & "C Over " & Duration_seconds & " Seconds"
    End Function

End Structure

