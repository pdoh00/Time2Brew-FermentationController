Imports System.IO

Public Structure PROFILE_STEP
    Public StartTemperature_C As Single
    Public EndTemperature_C As Single

    Public Duration_seconds As UInt32
    Public Function Serialize() As Byte()
        Dim temp As UInt16
        Using ms As New MemoryStream()
            temp = (StartTemperature_C * 10)
            ms.Write(BitConverter.GetBytes(temp), 0, 2)
            temp = (EndTemperature_C * 10)
            ms.Write(BitConverter.GetBytes(temp), 0, 2)
            ms.Write(BitConverter.GetBytes(Duration_seconds), 0, 4)
            Return ms.ToArray()
        End Using
    End Function

    Public Sub New(sourceData As Byte(), sourceOffset As Integer)
        Dim offset As Integer = sourceOffset
        StartTemperature_C = CSng(BitConverter.ToInt16(sourceData, offset)) * 0.1
        offset += 2
        EndTemperature_C = CSng(BitConverter.ToInt16(sourceData, offset)) * 0.1
        offset += 2
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

