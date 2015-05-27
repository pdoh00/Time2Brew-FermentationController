Imports System.Net
Imports System.IO
Imports Microsoft.Win32

Class MainWindow
    Private Controller As FermentationControllerDevice
    Private coms As IHttpCommsProvider
    Private poll As New Timers.Timer(1000)
    Private pollTick As Integer = 0
    Private Sub MainWindow_Loaded(sender As Object, e As RoutedEventArgs) Handles Me.Loaded
        Dim ssdp As New Discovery.SSDP.Agents.ClientAgent()
        coms = New HttpCommsProviderWebClient
        Dim IPAddress As String = InputBox("Controller IP Address:", "", "192.168.0.109")

        Controller = New FermentationControllerDevice(coms, "http://" & IPAddress)

        AddHandler poll.Elapsed, Async Sub()
                                     Try
                                         Dim temp1 = Await Controller.GetTemperature(0)
                                         Dim temp2 = Await Controller.GetTemperature(1)
                                         pollTick += 1
                                         Using fs As New StreamWriter("C:\poll.csv", True)
                                             fs.WriteLine(pollTick & "," & temp1 & "," & temp2)
                                         End Using
                                         Dispatcher.Invoke(Sub()
                                                               Me.Title = pollTick & ":  Probe0=" & temp1 & "C  Probe1=" & temp2 & "C"
                                                           End Sub)
                                     Catch ex As Exception

                                     End Try
                                     'poll.Enabled = False

                                     'poll.Enabled = True
                                 End Sub

    End Sub

    Private Async Sub cmdGetTemp_Click(sender As Object, e As RoutedEventArgs) Handles cmdGetTemp.Click
        response.Text = "--"
        Try
            Dim temp0 = Await Controller.GetTemperature(0)
            Dim temp1 = Await Controller.GetTemperature(1)
            response.Text = "Probe0: " & temp0 & vbCrLf & "Probe1: " & temp1
        Catch ex As Exception
            response.Text = "Error:" & ex.ToString
        End Try

    End Sub

    Private Async Sub cmdTrimFS_Click(sender As Object, e As RoutedEventArgs) Handles cmdTrimFS.Click
        response.Text = "--"
        Try
            Await Controller.TrimFileSystem()
            response.Text = "OK..."
        Catch ex As Exception
            response.Text = "Error:" & ex.ToString
        End Try

    End Sub

    Private Async Sub cmdFormatFS_Click(sender As Object, e As RoutedEventArgs) Handles cmdFormatFS.Click
        response.Text = "--"
        Try
            Await Controller.WipeAndFactoryDefault()
            response.Text = "OK..."
        Catch ex As Exception
            response.Text = "Error:" & ex.ToString
        End Try
    End Sub

    Private Async Sub cmdSetEquipmentProfile_Click(sender As Object, e As RoutedEventArgs) Handles cmdSetEquipmentProfile.Click
        Try
            response.Text = "--"
            Dim profiles = Await Controller.GetEquipmentProfileListing()
            Dim prflSelector = New ProfileSelector(profiles)
            prflSelector.ShowDialog()
            If prflSelector.isCanceled Then Return
            Dim name = prflSelector.SelectedProfile
            prflSelector = Nothing
             Try
                Await Controller.SetEquipmentProfile(name)
                response.Text = "OK"
            Catch ex As Exception
                response.Text = "Error:" & ex.ToString
            End Try
        Catch ex As Exception
            response.Text = ex.ToString
            Return
        End Try
    End Sub

    Private Async Sub cmdGetEquipmentProfileListing_Click(sender As Object, e As RoutedEventArgs) Handles cmdGetEquipmentProfileListing.Click
        response.Text = "--"
        Try
            Dim eq = Await Controller.GetEquipmentProfileListing()
            response.Text = ""
            For Each itm In eq
                response.Text += itm & vbCrLf
            Next
        Catch ex As Exception
            response.Text = "Error:" & ex.ToString
        End Try

    End Sub

    Private Async Sub cmdUploadEquipmentProfile_Click(sender As Object, e As RoutedEventArgs) Handles cmdUploadEquipmentProfile.Click
        response.Text = "--"
        Dim name = InputBox("Equipment Profile Name")
        If name = "" Then Return
        Dim edt As New EquipmentEditor()
        edt.ShowDialog()
        If edt.Canceled Then Return

        Try
            Await Controller.UploadEquipmentProfile(name, edt.data)
            response.Text = "OK"
        Catch ex As Exception
            response.Text = "Error:" & ex.ToString
        End Try

    End Sub

    Private Async Sub cmdDownloadEquipmentProfile_Click(sender As Object, e As RoutedEventArgs) Handles cmdDownloadEquipmentProfile.Click
        Try
            response.Text = "--"
            Dim profiles = Await Controller.GetEquipmentProfileListing()
            Dim prflSelector = New ProfileSelector(profiles)
            prflSelector.ShowDialog()
            If prflSelector.isCanceled Then Return
            Dim name = prflSelector.SelectedProfile
            prflSelector = Nothing
            Try
                Dim temp = Await Controller.DownloadEquipmentProfile(name)
                Dim edt As New EquipmentEditor
                edt.Name = name
                edt.data = temp
                edt.ShowDialog()
                If edt.Canceled Then Return
                response.Text = temp.ToString
            Catch ex As Exception
                response.Text = "Error:" & ex.ToString
            End Try
        Catch ex As Exception
            response.Text = ex.ToString
            Return
        End Try
    End Sub

    Private Async Sub cmdRestart_Click(sender As Object, e As RoutedEventArgs) Handles cmdRestart.Click
        response.Text = "--"
        Try
            Await Controller.Reboot()
            response.Text = "OK"
        Catch ex As Exception
            response.Text = "Error:" & ex.ToString
        End Try
    End Sub

    Private Async Sub cmdGetStats_Click(sender As Object, e As RoutedEventArgs) Handles cmdGetStats.Click
        response.Text = "--"
        Try
            Dim temp = Await Controller.GetStatus()
            response.Text = temp.ToString
        Catch ex As Exception
            response.Text = "Error:" & ex.ToString
        End Try

    End Sub

    Private Async Sub cmdGetProfileListing_Click(sender As Object, e As RoutedEventArgs) Handles cmdGetProfileListing.Click
        response.Text = "--"
        Try
            Dim eq = Await Controller.GetProfileListing()
            response.Text = ""
            For Each itm In eq
                response.Text += itm & vbCrLf
            Next
        Catch ex As Exception
            response.Text = "Error:" & ex.ToString
        End Try

    End Sub

    Private Async Sub cmdUploadProfile_Click(sender As Object, e As RoutedEventArgs) Handles cmdUploadProfile.Click
        response.Text = "--"
        Dim l As New List(Of PROFILE_STEP)
        Dim rnd As New Random(Now.Millisecond)

        l.Add(New PROFILE_STEP(20, 30, 604800)) '7 days
        l.Add(New PROFILE_STEP(30, 30, 240)) '4 minutes
        l.Add(New PROFILE_STEP(30, 10, 600)) '10 minutes
        l.Add(New PROFILE_STEP(10, 10, 120)) '2 minutes
        l.Add(New PROFILE_STEP(10, 12, 120)) '2 minutes
        l.Add(New PROFILE_STEP(12, 50, 600)) '10 minutes

        Try
            Await Controller.UploadProfile("testProfile2", l)
            response.Text = "OK"
        Catch ex As Exception
            response.Text = "Error:" & ex.ToString
        End Try


    End Sub

    Private Async Sub cmdDownloadProfile_Click(sender As Object, e As RoutedEventArgs) Handles cmdDownloadProfile.Click
        Try
            response.Text = "--"
            Dim profiles = Await Controller.GetProfileListing()
            Dim prflSelector = New ProfileSelector(profiles)
            prflSelector.ShowDialog()
            If prflSelector.isCanceled Then Return
            Dim name = prflSelector.SelectedProfile
            prflSelector = Nothing
            Try
                Dim lst = Await Controller.DownloadProfile(name)
                response.Text = ""
                For Each itm In lst
                    response.Text += itm.ToString & vbCrLf
                Next
            Catch ex As Exception
                response.Text = "Error:" & ex.ToString
            End Try
        Catch ex As Exception
            response.Text = ex.ToString
            Return
        End Try
    End Sub

    Private Async Sub cmdExecProfile_Click(sender As Object, e As RoutedEventArgs) Handles cmdExecProfile.Click
        Try
            response.Text = "--"
            Dim profiles = Await Controller.GetProfileListing()
            Dim prflSelector = New ProfileSelector(profiles)
            prflSelector.ShowDialog()
            If prflSelector.isCanceled Then Return
            Dim name = prflSelector.SelectedProfile
            prflSelector = Nothing
            Try
                Await Controller.ExecuteProfile(name)
                response.Text = "OK"
            Catch ex As Exception
                response.Text = "Error:" & ex.ToString
            End Try
        Catch ex As Exception
            response.Text = ex.ToString
            Return
        End Try

    End Sub

    Private Async Sub cmdTerminateProfile_Click(sender As Object, e As RoutedEventArgs) Handles cmdTerminateProfile.Click
        response.Text = "--"
        Try
            Await Controller.TerminateProfile()
            response.Text = "OK"
        Catch ex As Exception
            response.Text = "Error:" & ex.ToString
        End Try


    End Sub

    Private Async Sub cmdGetInstanceListing_Click(sender As Object, e As RoutedEventArgs) Handles cmdGetInstanceListing.Click
        Try
            response.Text = "--"
            Dim profiles = Await Controller.GetProfileListing()
            Dim prflSelector = New ProfileSelector(profiles)
            prflSelector.ShowDialog()
            If prflSelector.isCanceled Then Return
            Dim name = prflSelector.SelectedProfile
            prflSelector = Nothing

            Try
                Dim lst = Await Controller.GetInstanceListing(name)
                response.Text = ""
                For Each itm In lst
                    response.Text += itm.ToString & vbCrLf
                Next
            Catch ex As Exception
                response.Text = "Error:" & ex.ToString
            End Try
        Catch ex As Exception
            response.Text = ex.ToString
            Return
        End Try

    End Sub

    Private Async Sub cmdDownloadInstance_Click(sender As Object, e As RoutedEventArgs) Handles cmdDownloadInstance.Click
        Try
            response.Text = "--"
            Dim profiles = Await Controller.GetProfileListing()
            Dim prflSelector = New ProfileSelector(profiles)
            prflSelector.ShowDialog()
            If prflSelector.isCanceled Then Return
            Dim name = prflSelector.SelectedProfile
            prflSelector = Nothing

            Try
                Dim lst = Await Controller.GetInstanceListing(name)

                Dim selector As New InstanceSelector(lst)
                selector.ShowDialog()
                If selector.isCanceled Then Return

                Dim ret = Await Controller.DownloadInstance(name, selector.SelectedDate)
                response.Text = ""
                For Each itm In ret
                    response.Text += itm.ToString & vbCrLf
                Next

            Catch ex As Exception
                response.Text = "Error:" & ex.ToString
                Return
            End Try
        Catch ex As Exception
            response.Text = ex.ToString
            Return
        End Try
    End Sub

    Private Async Sub cmdDownloadTrend_Click(sender As Object, e As RoutedEventArgs) Handles cmdDownloadTrend.Click
        Try
            response.Text = "--"
            Dim profiles = Await Controller.GetProfileListing()
            Dim prflSelector = New ProfileSelector(profiles)
            prflSelector.ShowDialog()
            If prflSelector.isCanceled Then Return
            Dim name = prflSelector.SelectedProfile
            prflSelector = Nothing

            Try
                Dim lst = Await Controller.GetInstanceListing(name)

                Dim selector As New InstanceSelector(lst)
                selector.ShowDialog()
                If selector.isCanceled Then Return

                Dim ret = Await Controller.DownloadTrendData(name, selector.SelectedDate)

                Dim fod = New SaveFileDialog
                fod.Filter = "Comma Seperated Values (*.csv)|*.csv"
                fod.AddExtension = False
                fod.ShowDialog()
                Using sw = New StreamWriter(fod.OpenFile())
                    sw.WriteLine("Probe0, Probe1, SetPoint, Output")
                    For Each itm In ret
                        sw.WriteLine(itm.ToString)
                    Next
                End Using
                response.Text = "OK"

            Catch ex As Exception
                response.Text = "Error:" & ex.ToString
                Return
            End Try
        Catch ex As Exception
            response.Text = ex.ToString
            Return
        End Try

    End Sub

    Private Async Sub cmdDeleteEquipmentProfile_Click(sender As Object, e As RoutedEventArgs) Handles cmdDeleteEquipmentProfile.Click
        Try
            response.Text = "--"
            Dim EquipmentProfiles = Await Controller.GetEquipmentProfileListing()
            Dim prflSelector = New ProfileSelector(EquipmentProfiles)
            prflSelector.ShowDialog()
            If prflSelector.isCanceled Then Return
            Dim name = prflSelector.SelectedProfile
            prflSelector = Nothing
            Try
                Await Controller.DeleteEquipmentProfile(name)
                response.Text = "OK"
            Catch ex As Exception
                response.Text = "Error:" & ex.ToString
            End Try
        Catch ex As Exception
            response.Text = ex.ToString
            Return
        End Try
    End Sub

    Private Async Sub cmdDeleteProfile_Click(sender As Object, e As RoutedEventArgs) Handles cmdDeleteProfile.Click
        Try
            response.Text = "--"
            Dim profiles = Await Controller.GetProfileListing()
            Dim prflSelector = New ProfileSelector(profiles)
            prflSelector.ShowDialog()
            If prflSelector.isCanceled Then Return
            Dim name = prflSelector.SelectedProfile
            prflSelector = Nothing
            Try
                Await Controller.DeleteProfile(name)
                response.Text = "OK"
            Catch ex As Exception
                response.Text = "Error:" & ex.ToString
            End Try
        Catch ex As Exception
            response.Text = ex.ToString
            Return
        End Try
    End Sub

    Private Async Sub cmdDeleteTrend_Click(sender As Object, e As RoutedEventArgs) Handles cmdDeleteTrend.Click
        Try
            response.Text = "--"
            Dim profiles = Await Controller.GetProfileListing()
            Dim prflSelector = New ProfileSelector(profiles)
            prflSelector.ShowDialog()
            If prflSelector.isCanceled Then Return
            Dim name = prflSelector.SelectedProfile
            prflSelector = Nothing
            Try
                Dim lst = Await Controller.GetInstanceListing(name)

                Dim selector As New InstanceSelector(lst)
                selector.ShowDialog()
                If selector.isCanceled Then Return

                Await Controller.DeleteInstance(name, selector.SelectedDate)
                response.Text = "OK"
            Catch ex As Exception
                response.Text = "Error:" & ex.ToString
            End Try
        Catch ex As Exception
            response.Text = ex.ToString
            Return
        End Try
    End Sub

    Private Async Sub cmdFirmware_Click(sender As Object, e As RoutedEventArgs) Handles cmdFirmware.Click
        Dim dlg As New Forms.FolderBrowserDialog
        dlg.ShowDialog()
        Try
            response.Text = "--"
            Try
                Using binFirmware = HEX_FirmwareProcessor.GenerateMasterBIN(dlg.SelectedPath)
                    Await Controller.uploadfirmware(binFirmware)
                    response.Text = "OK"
                End Using
            Catch ex As Exception
                response.Text = "Error:" & ex.ToString
            End Try

        Catch ex As Exception

        End Try
    End Sub

    Private Async Sub cmdVersion_Click(sender As Object, e As RoutedEventArgs) Handles cmdVersion.Click
        response.Text = "--"
        Try
            response.Text = Await Controller.GetVersion
        Catch ex As Exception
            response.Text = "Error:" & ex.ToString
        End Try


    End Sub

    Private Sub cmdTogglePolling_Click(sender As Object, e As RoutedEventArgs) Handles cmdTogglePolling.Click
        poll.Enabled = Not poll.Enabled
    End Sub

    Private Async Sub cmdUploadFile_Click(sender As Object, e As RoutedEventArgs) Handles cmdUploadFile.Click
        Dim dlg As New OpenFileDialog
        dlg.ShowDialog()

        Using inp = dlg.OpenFile
            Dim fname = Path.GetFileName(dlg.FileName)
            Await Controller.uploadFile(inp, fname)
        End Using
    End Sub
End Class

Public Class HttpCommsProviderWebClient
    Implements IHttpCommsProvider

    Public Async Function Comms_GET(URL As String) As System.Threading.Tasks.Task(Of HTTP_Comms_Result) Implements IHttpCommsProvider.Comms_GET
        Dim st As New Stopwatch
        Dim request As HttpWebRequest = Nothing
        For retry = 1 To 3
            Try
                request = WebRequest.Create(URL)
                request.Pipelined = False
                request.ServicePoint.ConnectionLeaseTimeout = 200
                request.ServicePoint.UseNagleAlgorithm = False
                request.ServicePoint.Expect100Continue = False
                request.Timeout = 1000
                request.Method = "GET"
                request.ContentLength = 0

                st.Start()
                Using response As HttpWebResponse = Await request.GetResponseAsync()
                    Using rsp = response.GetResponseStream
                        Using body As New MemoryStream
                            Dim buff(512) As Byte
                            While body.Length < response.ContentLength
                                Console.WriteLine("Body.Length=" & body.Length & " Response.ContentLength=" & response.ContentLength)
                                Dim bytesRead = Await rsp.ReadAsync(buff, 0, buff.Length)
                                Console.WriteLine("---BytesRead=" & bytesRead)
                                If bytesRead > 0 Then body.Write(buff, 0, bytesRead)
                            End While
                            st.Stop()
                            Console.WriteLine("Time=" & st.ElapsedMilliseconds & "ms")
                            Return New HTTP_Comms_Result(response.StatusCode, body.ToArray)
                        End Using
                    End Using
                End Using

            Catch tm As TimeoutException
                If retry = 3 Then
                    Throw tm
                Else
                    If IsNothing(request) = False Then request.Abort()
                    System.Threading.Thread.Sleep(100)
                End If
            Catch we As WebException
                Dim ret As New HTTP_Comms_Result()
                Dim resp As HttpWebResponse = we.Response
                ret.StatusCode = resp.StatusCode

                Using rsp = resp.GetResponseStream
                    Dim dat(resp.ContentLength - 1) As Byte
                    Dim bRemain = resp.ContentLength
                    ret.Body = dat
                    While (bRemain)
                        bRemain -= rsp.Read(ret.Body, 0, resp.ContentLength)
                    End While
                End Using
                Return ret
            Catch ex As Exception
                Console.WriteLine(ex.ToString)
            End Try

        Next
        Return Nothing
    End Function

    Public Async Function Comms_PUT(URL As String) As System.Threading.Tasks.Task(Of HTTP_Comms_Result) Implements IHttpCommsProvider.Comms_PUT
        Dim request As HttpWebRequest = Nothing
        Dim st As New Stopwatch
        For retry = 1 To 3
            Try
                request = WebRequest.Create(URL)
                request.Pipelined = False
                request.ServicePoint.ConnectionLeaseTimeout = 200
                request.ServicePoint.UseNagleAlgorithm = False
                request.ServicePoint.Expect100Continue = False
                request.Timeout = 5000
                request.Method = "PUT"
                request.ContentLength = 0

                st.Start()
                Using response As HttpWebResponse = Await request.GetResponseAsync()
                    Using rsp = response.GetResponseStream
                        Using body As New MemoryStream
                            Dim buff(512) As Byte
                            While body.Length < response.ContentLength
                                Dim bytesRead = Await rsp.ReadAsync(buff, 0, buff.Length)
                                If bytesRead > 0 Then body.Write(buff, 0, bytesRead)
                            End While
                            st.Stop()
                            Console.WriteLine("Time=" & st.ElapsedMilliseconds & "ms")
                            Return New HTTP_Comms_Result(response.StatusCode, body.ToArray)
                        End Using
                    End Using
                End Using
            Catch tm As TimeoutException
                If retry = 3 Then
                    If IsNothing(request) = False Then request.Abort()
                    System.Threading.Thread.Sleep(100)
                    Throw tm
                Else
                    If IsNothing(request) = False Then request.Abort()
                    System.Threading.Thread.Sleep(100)
                End If
            Catch we As WebException
                If we.Status = WebExceptionStatus.Timeout Then
                    If retry = 3 Then
                        Throw New TimeoutException
                    Else
                        If IsNothing(request) = False Then request.Abort()
                        System.Threading.Thread.Sleep(1000)
                    End If
                Else
                    Dim ret As New HTTP_Comms_Result()
                    Dim resp As HttpWebResponse = we.Response
                    ret.StatusCode = resp.StatusCode
                    Using rsp = resp.GetResponseStream
                        Dim dat(resp.ContentLength - 1) As Byte
                        ret.Body = dat
                        rsp.Read(ret.Body, 0, resp.ContentLength)
                    End Using
                    If IsNothing(request) = False Then request.Abort()
                    System.Threading.Thread.Sleep(1000)
                    Return ret
                End If
            Catch ex As Exception
                Debug.WriteLine(ex.ToString)
            End Try


        Next
        Return Nothing
    End Function
End Class
