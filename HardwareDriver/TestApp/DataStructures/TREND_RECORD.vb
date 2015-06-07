Public Structure TREND_RECORD
    Public Duration As Integer
    Public Probe0_Temperature_C As Single
    Public Probe1_Temperature_C As Single
    Public Output_percent As Single
    Public RelayState As Byte

    Public Sub New(sourceData As Byte(), sourceOffset As Integer)
        Dim offset As Integer = sourceOffset
        Probe0_Temperature_C = CSng(BitConverter.ToInt16(sourceData, offset)) * 0.25
        offset += 2
        Probe1_Temperature_C = CSng(BitConverter.ToInt16(sourceData, offset)) * 0.25
        offset += 2
        Output_percent = CSng(sourceData(offset))
        If Output_percent >= 0 AndAlso Output_percent <= 100 Then
            Output_percent *= 0.01
        Else
            Output_percent = Output_percent - 256
            Output_percent *= 0.01
        End If
        offset += 1
        RelayState = sourceData(offset)
        offset += 1
        Duration = sourceData(offset)
        offset += 1
    End Sub

    Public Overrides Function ToString() As String
        Return Probe0_Temperature_C & ", " & Probe1_Temperature_C & "," & Output_percent & "," & If(RelayState = 1, "1", If(RelayState = 2, "-1", "0"))
    End Function
End Structure
