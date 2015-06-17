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
    Public CoolWhenCanTurnOff As UInt32
    Public CoolWhenCanTurnOn As UInt32
    Public HeatWhenCanTurnOff As UInt32
    Public HeatWhenCanTurnOn As UInt32
    Public Output As Integer
    Public ProcessPID_ITerm As Single
    Public ProcessPID_error As Single
    Public TargetPID_ITerm As Single
    Public TargetPID_error As Single
    Public ProfileStartTime As UInt32
    Public TimeTurnedOn As UInt32
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
        Probe0Temperature_C = CDbl(BitConverter.ToSingle(sourceData, offset))
        offset += 4
        Probe1Assigment = DirectCast(sourceData(offset), PROBE_ASSIGNMENT)
        offset += 1
        Probe1Temperature_C = CDbl(BitConverter.ToSingle(sourceData, offset))
        offset += 4
        HeatRelay_On = sourceData(offset)
        offset += 1
        CoolRelay_On = sourceData(offset)
        offset += 1
        ActiveProfileName = GetString(sourceData, offset, 64)
        offset += 64
        StepIdx = BitConverter.ToUInt16(sourceData, offset)
        offset += 2
        StepTemperature_C = CDbl(BitConverter.ToSingle(sourceData, offset))
        offset += 4
        StepTimeRemaining_seconds = BitConverter.ToUInt32(sourceData, offset)
        offset += 4
        ManualSetPoint_C = CDbl(BitConverter.ToSingle(sourceData, offset))
        offset += 4
        ProfileStartTime_seconds = BitConverter.ToUInt32(sourceData, offset)
        offset += 4

        EquipmentProfileName = GetString(sourceData, offset, 64)
        offset += 64

        CoolWhenCanTurnOff = BitConverter.ToUInt32(sourceData, offset)
        offset += 4
        CoolWhenCanTurnOn = BitConverter.ToUInt32(sourceData, offset)
        offset += 4
        HeatWhenCanTurnOff = BitConverter.ToUInt32(sourceData, offset)
        offset += 4
        HeatWhenCanTurnOn = BitConverter.ToUInt32(sourceData, offset)
        offset += 4

        Output = sourceData(offset)
        If (Output > 128) Then Output = Output - 256
        offset += 1

        ProcessPID_ITerm = BitConverter.ToSingle(sourceData, offset)
        offset += 4
        ProcessPID_error = BitConverter.ToSingle(sourceData, offset)
        offset += 4
        TargetPID_ITerm = BitConverter.ToSingle(sourceData, offset)
        offset += 4
        TargetPID_error = BitConverter.ToSingle(sourceData, offset)
        offset += 4

        ProfileStartTime = BitConverter.ToUInt32(sourceData, offset)
        offset += 4

        TimeTurnedOn = BitConverter.ToUInt32(sourceData, offset)
        offset += 4

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
        ret += "CoolWhenCanTurnOff: " & New DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc).AddSeconds(Me.CoolWhenCanTurnOff).ToLocalTime & vbCrLf
        ret += "CoolWhenCanTurnOn: " & New DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc).AddSeconds(Me.CoolWhenCanTurnOn).ToLocalTime & vbCrLf
        ret += "HeatWhenCanTurnOff: " & New DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc).AddSeconds(Me.HeatWhenCanTurnOff).ToLocalTime & vbCrLf
        ret += "HeatWhenCanTurnOn: " & New DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc).AddSeconds(Me.HeatWhenCanTurnOn).ToLocalTime & vbCrLf
        ret += "Output: " & Me.Output & vbCrLf
        ret += "ProcessPID_ITerm: " & Me.ProcessPID_ITerm.ToString("0.000") & vbCrLf
        ret += "ProcessPID_error: " & Me.ProcessPID_error.ToString("0.000") & vbCrLf
        ret += "TargetPID_ITerm: " & Me.TargetPID_ITerm.ToString("0.000") & vbCrLf
        ret += "TargetPID_error: " & Me.TargetPID_error.ToString("0.000") & vbCrLf
        ret += "TimeTurnedOn: " & New DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc).AddSeconds(Me.TimeTurnedOn).ToLocalTime & vbCrLf

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


