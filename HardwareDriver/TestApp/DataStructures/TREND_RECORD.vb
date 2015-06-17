Public Structure TREND_RECORD
    Public Probe0_Temperature_C As Single
    Public Probe1_Temperature_C As Single
    Public Output_percent As Single
    Public RelayState As Byte

    Public Sub New(sourceData As Byte(), sourceOffset As Integer)
        Dim sample As UInt32 = BitConverter.ToUInt32(sourceData, sourceOffset)
        RelayState = sample And 3
        sample >>= 2
        Output_percent = sample And 1023
        sample >>= 10
        Probe1_Temperature_C = sample And 1023
        sample >>= 10
        Probe0_Temperature_C = sample And 1023

        Probe0_Temperature_C *= 0.146628
        Probe1_Temperature_C *= 0.146628

        Probe0_Temperature_C -= 25
        Probe1_Temperature_C -= 25
        Output_percent -= 100
      
    End Sub

    Public Overrides Function ToString() As String
        Return Probe0_Temperature_C & ", " & Probe1_Temperature_C & "," & Output_percent & "," & If(RelayState = 1, "1", If(RelayState = 2, "-1", "0"))
    End Function
End Structure
