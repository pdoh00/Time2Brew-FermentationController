Public Structure CONTROLLER_STATUS

    Public SystemTime As DateTime
    Public SystemMode As SYSTEMMODE
    Public RegulationMode As REGULATIONMODE
    Public Probe0Assigment As PROBE_ASSIGNMENT
    Public Probe0Temperature_C As Double
    Public Probe1Assigment As PROBE_ASSIGNMENT
    Public Probe1Temperature_C As Double
    Public HeatRelay_On As Byte
    Public CoolRelay_On As Byte
    Public ActiveProfileName As String
    Public StepIdx As UInt16
    Public StepTemperature_C As Double
    Public StepTimeRemaining_seconds As UInt32
    Public ManualSetPoint_C As Double
    Public EquipmentProfileName As String

    Public ProfileStartTime_seconds As UInt32
    Public Sub New(sourceData As Byte())
        Dim offset As Integer = 0
        Dim timeOffset = BitConverter.ToUInt32(sourceData, offset)
        offset += 4
        SystemTime = New DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc).AddSeconds(timeOffset).ToLocalTime

        SystemMode = DirectCast(sourceData(offset), SYSTEMMODE)
        offset += 1

        RegulationMode = DirectCast(sourceData(offset), REGULATIONMODE)
        offset += 1

        Probe0Assigment = DirectCast(sourceData(offset), PROBE_ASSIGNMENT)
        offset += 1
        Probe0Temperature_C = CDbl(BitConverter.ToInt16(sourceData, offset)) * 0.1
        offset += 2
        Probe1Assigment = DirectCast(sourceData(offset), PROBE_ASSIGNMENT)
        offset += 1
        Probe1Temperature_C = CDbl(BitConverter.ToInt16(sourceData, offset)) * 0.1
        offset += 2
        HeatRelay_On = sourceData(offset)
        offset += 1
        CoolRelay_On = sourceData(offset)
        offset += 1
        ActiveProfileName = GetString(sourceData, offset, 64)
        offset += 64
        StepIdx = BitConverter.ToUInt16(sourceData, offset)
        offset += 2
        StepTemperature_C = CDbl(BitConverter.ToInt16(sourceData, offset)) * 0.1
        offset += 2
        StepTimeRemaining_seconds = BitConverter.ToUInt32(sourceData, offset)
        offset += 4
        ManualSetPoint_C = CDbl(BitConverter.ToInt16(sourceData, offset)) * 0.1
        offset += 2
        ProfileStartTime_seconds = BitConverter.ToUInt32(sourceData, offset)
        offset += 4

        EquipmentProfileName = GetString(sourceData, offset, 64)
        offset += 64
    End Sub

    Public Overrides Function ToString() As String
        Dim ret As String = ""
        ret += "SystemTime: " & Me.SystemTime.ToString & vbCrLf
        ret += "SystemMode: " & Me.SystemMode.ToString & vbCrLf
        ret += "RegulationMode: " & Me.RegulationMode.ToString & vbCrLf
        ret += "Probe0Assigment: " & Me.Probe0Assigment.ToString & vbCrLf
        ret += "Probe0Temperature_C: " & Me.Probe0Temperature_C.ToString & vbCrLf
        ret += "Probe1Assigment: " & Me.Probe1Assigment.ToString & vbCrLf
        ret += "Probe1Temperature_C: " & Me.Probe1Temperature_C.ToString & vbCrLf
        ret += "HeatRelay_On: " & Me.HeatRelay_On.ToString & vbCrLf
        ret += "CoolRelay_On: " & Me.CoolRelay_On.ToString & vbCrLf
        ret += "ActiveProfileName: " & Me.ActiveProfileName & vbCrLf
        ret += "StepIdx: " & Me.StepIdx.ToString & vbCrLf
        ret += "StepTemperature_C: " & Me.StepTemperature_C.ToString & vbCrLf
        ret += "StepTimeRemaining_seconds: " & Me.StepTimeRemaining_seconds.ToString & vbCrLf
        ret += "ManualSetPoint_C: " & Me.ManualSetPoint_C.ToString & vbCrLf
        ret += "EquipmentProfileName: " & Me.EquipmentProfileName & vbCrLf
        Return ret
    End Function

    Private Function GetString(dat() As Byte, offset As Integer, maxLen As Integer) As String
        Dim realLength As Integer = 0
        For idx = offset To offset + maxLen
            If dat(idx) = 0 Then
                Exit For
            Else
                realLength += 1
            End If
        Next
        Return System.Text.UTF8Encoding.UTF8.GetString(dat, offset, realLength)
    End Function
End Structure


