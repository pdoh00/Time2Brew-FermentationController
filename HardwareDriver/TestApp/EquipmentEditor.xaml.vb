
Public Class EquipmentEditor
    Public data As EQUIPMENT_PROFILE = New EQUIPMENT_PROFILE
    Public Canceled As Boolean = True
    Public ProfileName As String
    Public EquipmentID As String = ""

    Private Sub EquipmentEditor_Activated(sender As Object, e As EventArgs) Handles Me.Loaded
        Me.EquipmentName.Text = data.EquipmentName
        Me.RegulationMode.Text = data.RegulationMode
        Me.Probe0Assignment.Text = data.Probe0Assignment
        Me.Probe1Assignment.Text = data.Probe1Assignment
        Me.HeatMinTimeOn_seconds.Text = data.HeatMinTimeOn_seconds
        Me.HeatMinTimeOff_seconds.Text = data.HeatMinTimeOff_seconds
        Me.CoolMinTimeOn_seconds.Text = data.CoolMinTimeOn_seconds
        Me.CoolMinTimeOff_seconds.Text = data.CoolMinTimeOff_seconds

        Me.Process_Kp.Text = data.Process_Kp
        Me.Process_Ki.Text = data.Process_Ki
        Me.Process_Kd.Text = data.Process_Kd

        Me.Target_Kp.Text = data.Target_Kp
        Me.Target_Ki.Text = data.Target_Ki
        Me.Target_Kd.Text = data.Target_Kd

        Me.TargetOutput_Max_C.Text = data.TargetOutput_Max_C
        Me.TargetOutput_Min_C.Text = data.TargetOutput_Min_C

        Me.coolDifferential.Text = data.coolDifferential
        Me.coolTransition.Text = data.coolTransition
        Me.heatDifferential.Text = data.heatDifferential
        Me.heatTransition.Text = data.heatTransition

        Me.Process_D_AdaptiveBand.Text = data.Process_D_AdaptiveBand
        Me.Process_D_FilterCoeff.Text = data.Process_D_FilterCoeff
        Me.Process_D_FilterGain.Text = data.Process_D_FilterGain

        Me.Target_D_AdaptiveBand.Text = data.Target_D_AdaptiveBand
        Me.Target_D_FilterCoeff.Text = data.Target_D_FilterCoeff
        Me.Target_D_FilterGain.Text = data.Target_D_FilterGain


    End Sub

    Private Sub cmdOK_Click(sender As Object, e As RoutedEventArgs) Handles cmdOK.Click
        data.RegulationMode = Me.RegulationMode.Text
        data.Probe0Assignment = Me.Probe0Assignment.Text
        data.Probe1Assignment = Me.Probe1Assignment.Text
        data.HeatMinTimeOn_seconds = Me.HeatMinTimeOn_seconds.Text
        data.HeatMinTimeOff_seconds = Me.HeatMinTimeOff_seconds.Text
        data.CoolMinTimeOn_seconds = Me.CoolMinTimeOn_seconds.Text
        data.CoolMinTimeOff_seconds = Me.CoolMinTimeOff_seconds.Text

        data.Process_Kp = Me.Process_Kp.Text
        data.Process_Ki = Me.Process_Ki.Text
        data.Process_Kd = Me.Process_Kd.Text

        data.Target_Kp = Me.Target_Kp.Text
        data.Target_Ki = Me.Target_Ki.Text
        data.Target_Kd = Me.Target_Kd.Text

        data.TargetOutput_Max_C = Me.TargetOutput_Max_C.Text
        data.TargetOutput_Min_C = Me.TargetOutput_Min_C.Text

        data.coolDifferential = Me.coolDifferential.Text
        data.coolTransition = Me.coolTransition.Text
        data.heatDifferential = Me.heatDifferential.Text
        data.heatTransition = Me.heatTransition.Text

        data.Process_D_AdaptiveBand = Me.Process_D_AdaptiveBand.Text
        data.Process_D_FilterCoeff = Me.Process_D_FilterCoeff.Text
        data.Process_D_FilterGain = Me.Process_D_FilterGain.Text

        data.Target_D_AdaptiveBand = Me.Target_D_AdaptiveBand.Text
        data.Target_D_FilterCoeff = Me.Target_D_FilterCoeff.Text
        data.Target_D_FilterGain = Me.Target_D_FilterGain.Text

        data.EquipmentName = Me.EquipmentName.Text
        Me.Canceled = False
        Me.Close()
    End Sub

    Private Sub cmdCancel_Click(sender As Object, e As RoutedEventArgs) Handles cmdCancel.Click
        Me.Canceled = True
        Me.Close()
    End Sub
End Class
