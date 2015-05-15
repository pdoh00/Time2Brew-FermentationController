Public Class InstanceSelector
    Public isCanceled As Boolean = True
    Public SelectedDate As DateTime

    Public Sub New(dtList As List(Of DateTime))

        ' This call is required by the designer.
        InitializeComponent()

        ' Add any initialization after the InitializeComponent() call.
        For Each itm In dtList
            lstInstances.Items.Add(itm)
        Next
    End Sub

    Private Sub cmdCancel_Click(sender As Object, e As RoutedEventArgs) Handles cmdCancel.Click
        isCanceled = True
        Me.Close()
    End Sub

    Private Sub cmdOK_Click(sender As Object, e As RoutedEventArgs) Handles cmdOK.Click
        isCanceled = False
        SelectedDate = lstInstances.SelectedItem
        Me.Close()
    End Sub
End Class
