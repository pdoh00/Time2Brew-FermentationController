Imports System.IO

Public Structure EQUIPMENT_PROFILE
    Public EquipmentName As String
    Public RegulationMode As REGULATIONMODE
    Public Probe0Assignment As PROBE_ASSIGNMENT
    Public Probe1Assignment As PROBE_ASSIGNMENT
    Public HeatMinTimeOn_seconds As UInt32
    Public HeatMinTimeOff_seconds As UInt32
    Public CoolMinTimeOn_seconds As UInt32
    Public CoolMinTimeOff_seconds As UInt32
    Public Process_Kp As Single
    Public Process_Ki As Single
    Public Process_Kd As Single
    Public Target_Kp As Single
    Public Target_Ki As Single
    Public Target_Kd As Single
    Public TargetOutput_Max_C As Single
    Public TargetOutput_Min_C As Single
    Public ThresholdDelta_C As Single
    Public Process_D_FilterGain As Single
    Public Process_D_FilterCoeff As Single
    Public Process_D_AdaptiveBand As Single
    Public Target_D_FilterGain As Single
    Public Target_D_FilterCoeff As Single
    Public Target_D_AdaptiveBand As Single
    Public coolDifferential As Single
    Public heatDifferential As Single
    Public coolTransition As Single
    Public heatTransition As Single

    Public Function Serialize() As Byte()
        Dim bb(63) As Byte
        For x = 0 To 63
            bb(x) = 0
        Next
        Dim nameBuff() As Byte = Text.ASCIIEncoding.ASCII.GetBytes(Me.EquipmentName)
        Array.Copy(nameBuff, 0, bb, 0, nameBuff.Length)

        Using ms As New MemoryStream()
            ms.Write(bb, 0, bb.Length)
            ms.WriteByte(CByte(RegulationMode))
            ms.WriteByte(CByte(Probe0Assignment))
            ms.WriteByte(CByte(Probe1Assignment))
            ms.WriteByte(0)
            ms.Write(BitConverter.GetBytes(HeatMinTimeOn_seconds), 0, 4)
            ms.Write(BitConverter.GetBytes(HeatMinTimeOff_seconds), 0, 4)
            ms.Write(BitConverter.GetBytes(CoolMinTimeOn_seconds), 0, 4)
            ms.Write(BitConverter.GetBytes(CoolMinTimeOff_seconds), 0, 4)
            ms.Write(BitConverter.GetBytes(Process_Kp), 0, 4)
            ms.Write(BitConverter.GetBytes(Process_Ki), 0, 4)
            ms.Write(BitConverter.GetBytes(Process_Kd), 0, 4)
            ms.Write(BitConverter.GetBytes(Target_Kp), 0, 4)
            ms.Write(BitConverter.GetBytes(Target_Ki), 0, 4)
            ms.Write(BitConverter.GetBytes(Target_Kd), 0, 4)
            ms.Write(BitConverter.GetBytes(TargetOutput_Max_C * 10), 0, 4)
            ms.Write(BitConverter.GetBytes(TargetOutput_Min_C * 10), 0, 4)
            ms.Write(BitConverter.GetBytes(Process_D_FilterGain), 0, 4)
            ms.Write(BitConverter.GetBytes(Process_D_FilterCoeff), 0, 4)
            ms.Write(BitConverter.GetBytes(Process_D_AdaptiveBand * 10), 0, 4)
            ms.Write(BitConverter.GetBytes(Target_D_FilterGain), 0, 4)
            ms.Write(BitConverter.GetBytes(Target_D_FilterCoeff), 0, 4)
            ms.Write(BitConverter.GetBytes(Target_D_AdaptiveBand * 10), 0, 4)
            ms.Write(BitConverter.GetBytes(CShort(coolDifferential * 10)), 0, 2)
            ms.Write(BitConverter.GetBytes(CShort(heatDifferential * 10)), 0, 2)
            ms.Write(BitConverter.GetBytes(CShort(coolTransition * 10)), 0, 2)
            ms.Write(BitConverter.GetBytes(CShort(heatTransition * 10)), 0, 2)
            Dim F16 = FletcherChecksum.Fletcher16(ms.ToArray, 0, ms.Length)
            ms.Position = ms.Length
            ms.Write(BitConverter.GetBytes(F16), 0, 2)
            Return ms.ToArray()
        End Using
    End Function

    Public Sub New(sourceData As Byte())
        Dim CalcF16 = FletcherChecksum.Fletcher16(sourceData, 0, sourceData.Length - 2)

        Dim offset As Integer = 0
        EquipmentName = GetString(sourceData, 0, 64)
        offset = 64
        RegulationMode = DirectCast(sourceData(offset), REGULATIONMODE)
        offset += 1
        Probe0Assignment = DirectCast(sourceData(offset), PROBE_ASSIGNMENT)
        offset += 1
        Probe1Assignment = DirectCast(sourceData(offset), PROBE_ASSIGNMENT)
        offset += 1
        offset += 1 'Dummy byte to account for padding on the dsPIC hardware
        HeatMinTimeOn_seconds = BitConverter.ToUInt32(sourceData, offset)
        offset += 4
        HeatMinTimeOff_seconds = BitConverter.ToUInt32(sourceData, offset)
        offset += 4
        CoolMinTimeOn_seconds = BitConverter.ToUInt32(sourceData, offset)
        offset += 4
        CoolMinTimeOff_seconds = BitConverter.ToUInt32(sourceData, offset)
        offset += 4
        Process_Kp = BitConverter.ToSingle(sourceData, offset)
        offset += 4
        Process_Ki = BitConverter.ToSingle(sourceData, offset)
        offset += 4
        Process_Kd = BitConverter.ToSingle(sourceData, offset)
        offset += 4
        Target_Kp = BitConverter.ToSingle(sourceData, offset)
        offset += 4
        Target_Ki = BitConverter.ToSingle(sourceData, offset)
        offset += 4
        Target_Kd = BitConverter.ToSingle(sourceData, offset)
        offset += 4
        TargetOutput_Max_C = BitConverter.ToSingle(sourceData, offset) * 0.1
        offset += 4
        TargetOutput_Min_C = BitConverter.ToSingle(sourceData, offset) * 0.1
        offset += 4
        Process_D_FilterGain = BitConverter.ToSingle(sourceData, offset)
        offset += 4
        Process_D_FilterCoeff = BitConverter.ToSingle(sourceData, offset)
        offset += 4
        Process_D_AdaptiveBand = BitConverter.ToSingle(sourceData, offset) * 0.1
        offset += 4
        Target_D_FilterGain = BitConverter.ToSingle(sourceData, offset)
        offset += 4
        Target_D_FilterCoeff = BitConverter.ToSingle(sourceData, offset)
        offset += 4
        Target_D_AdaptiveBand = BitConverter.ToSingle(sourceData, offset) * 0.1
        offset += 4
        coolDifferential = BitConverter.ToInt16(sourceData, offset) * 0.1
        offset += 2
        heatDifferential = BitConverter.ToInt16(sourceData, offset) * 0.1
        offset += 2
        coolTransition = BitConverter.ToInt16(sourceData, offset) * 0.1
        offset += 2
        heatTransition = BitConverter.ToInt16(sourceData, offset) * 0.1
        offset += 2
        Dim RecordF16 = BitConverter.ToUInt16(sourceData, offset)
        offset += 2

        If RecordF16 <> CalcF16 Then
            Throw New ArgumentException("Checksums to do not match Calculated=" & CalcF16 & " Record=" & RecordF16, "sourceData")
        End If

        Dim comp = Me.Serialize

        For idx = 0 To comp.Length - 1
            If comp(idx) <> sourceData(idx) Then
                Throw New ArgumentException("SelfSerialization does not match at byte #" & idx, "sourceData")
            End If
        Next

    End Sub

    Public Overrides Function ToString() As String
        Dim ret As String = ""
        ret = "Name:" & Me.EquipmentName & vbCrLf
        ret += "RegulationMode:" & Me.RegulationMode.ToString & vbCrLf
        ret += "Probe0Assignment:" & Me.Probe0Assignment.ToString & vbCrLf
        ret += "Probe1Assignment:" & Me.Probe1Assignment.ToString & vbCrLf
        ret += "HeatMinTimeOn_seconds:" & Me.HeatMinTimeOn_seconds.ToString & vbCrLf
        ret += "HeatMinTimeOff_seconds:" & Me.HeatMinTimeOff_seconds.ToString & vbCrLf
        ret += "CoolMinTimeOn_seconds:" & Me.CoolMinTimeOn_seconds.ToString & vbCrLf
        ret += "CoolMinTimeOff_seconds:" & Me.CoolMinTimeOff_seconds.ToString & vbCrLf
        ret += "Process_Kp:" & Me.Process_Kp.ToString & vbCrLf
        ret += "Process_Ki:" & Me.Process_Ki.ToString & vbCrLf
        ret += "Process_Kd:" & Me.Process_Kd.ToString & vbCrLf
        ret += "Target_Kp:" & Me.Target_Kp.ToString & vbCrLf
        ret += "Target_Ki:" & Me.Target_Ki.ToString & vbCrLf
        ret += "Target_Kd:" & Me.Target_Kd.ToString & vbCrLf
        ret += "TargetOutput_Max_C:" & Me.TargetOutput_Max_C.ToString & vbCrLf
        ret += "TargetOutput_Min_C:" & Me.TargetOutput_Min_C.ToString & vbCrLf
        ret += "ThresholdDelta_C:" & Me.ThresholdDelta_C.ToString & vbCrLf
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

