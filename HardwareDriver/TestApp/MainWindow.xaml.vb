Imports System.Net
Imports System.IO
Imports Microsoft.Win32
Imports System.Threading.Tasks

Class MainWindow
    Private Controller As FermentationControllerDevice
    Private coms As IHttpCommsProvider
    Private poll As New Timers.Timer(1000)
    Private pollTick As Integer = 0
    Private beaconer As BeaconDiscovery
    Private Sub MainWindow_Loaded(sender As Object, e As RoutedEventArgs) Handles Me.Loaded

        beaconer = New BeaconDiscovery(11624)
        AddHandler beaconer.OnDeviceDiscovered, AddressOf OnDeviceDiscovered


        Dim ssdp As New Discovery.SSDP.Agents.ClientAgent()
        coms = New HttpCommsProviderWebClient
        Dim IPAddress As String = InputBox("Controller IP Address:", "", My.Settings.LastAddress)
        My.Settings.LastAddress = IPAddress
        My.Settings.Save()

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

    Private Sub OnDeviceDiscovered(dev As Concurrent.ConcurrentBag(Of TCON_Device))
        Dispatcher.Invoke(Sub()
                              Devices.Items.Clear()
                              For Each d In dev
                                  Devices.Items.Add(d)
                              Next

                          End Sub)
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
            Dim name = prflSelector.SelectedProfile.Split(",")(0)
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
        Dim edt As New EquipmentEditor()
        edt.EquipmentID = ""
        edt.ShowDialog()
        If edt.Canceled Then Return

        Try
            Await Controller.UploadEquipmentProfile(Name, edt.data)
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
            Dim ID = prflSelector.SelectedProfile.Split(",")(0)
            prflSelector = Nothing
            Try
                Dim temp = Await Controller.DownloadEquipmentProfile(ID)
                Dim edt As New EquipmentEditor
                edt.EquipmentID = ID
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
        response.Text = "Restarting..."
        Try
            Await Controller.Reboot()
            response.Text = "Restarting...OK"
        Catch ex As Exception
            response.Text = "Restarting...Error:" & ex.ToString
        End Try
    End Sub

    Private Async Sub cmdGetStats_Click(sender As Object, e As RoutedEventArgs) Handles cmdGetStats.Click
        response.Text = "Getting Status..."
        Try
            Dim temp = Await Controller.GetStatus()
            response.Text = temp.ToString
        Catch ex As Exception
            response.Text = "Getting Status...Error:" & ex.ToString
        End Try

    End Sub

    Private Async Sub cmdGetProfileListing_Click(sender As Object, e As RoutedEventArgs) Handles cmdGetProfileListing.Click
        response.Text = "Getting Profile Listing...."
        Try
            Dim eq = Await Controller.GetProfileListing()
            response.Text = ""
            For Each itm In eq
                response.Text += itm & vbCrLf
            Next
        Catch ex As Exception
            response.Text = "Getting Profile Listing....Error:" & ex.ToString
        End Try

    End Sub

    Private Async Sub cmdUploadProfile_Click(sender As Object, e As RoutedEventArgs) Handles cmdUploadProfile.Click
        response.Text = "Uploading profile..."
        Dim l As New List(Of PROFILE_STEP)
        Dim rnd As New Random(Now.Millisecond)

        l.Add(New PROFILE_STEP(18.3, 18.3, 172800)) '2 Days
        l.Add(New PROFILE_STEP(18.3, 22, 86400)) '1 Day       
        l.Add(New PROFILE_STEP(22, 22, 1209600)) '14 Days

        Dim tprfl As New TEMPERATURE_PROFILE("Saison222", l)

        Try
            Await Controller.UploadProfile("", tprfl)
            response.Text = "Uploading profile...OK"
        Catch ex As Exception
            response.Text = "Uploading profile...Error:" & ex.ToString
        End Try


    End Sub

    Private Async Sub cmdDownloadProfile_Click(sender As Object, e As RoutedEventArgs) Handles cmdDownloadProfile.Click
        Try
            response.Text = "Downloading Profile..."
            Dim profiles = Await Controller.GetProfileListing()
            Dim prflSelector = New ProfileSelector(profiles)
            prflSelector.ShowDialog()
            If prflSelector.isCanceled Then Return
            Dim profileID = prflSelector.SelectedProfile.Split(",")(0)
            prflSelector = Nothing
            Try
                Dim profile = Await Controller.DownloadProfile(profileID)
                response.Text = profile.ToString
            Catch ex As Exception
                response.Text = "Downloading Profile...Error:" & ex.ToString
            End Try
        Catch ex As Exception
            response.Text = ex.ToString
            Return
        End Try
    End Sub

    Private Async Sub cmdExecProfile_Click(sender As Object, e As RoutedEventArgs) Handles cmdExecProfile.Click
        Try
            response.Text = "Executing Profile..."
            Dim profiles = Await Controller.GetProfileListing()
            Dim prflSelector = New ProfileSelector(profiles)
            prflSelector.ShowDialog()
            If prflSelector.isCanceled Then Return
            Dim profileID = prflSelector.SelectedProfile.Split(",")(0)
            prflSelector = Nothing
            Try
                Await Controller.ExecuteProfile(profileID)
                response.Text = "Executing Profile...OK"
            Catch ex As Exception
                response.Text = "Executing Profile...Error:" & ex.ToString
            End Try
        Catch ex As Exception
            response.Text = ex.ToString
            Return
        End Try

    End Sub

    Private Async Sub cmdTerminateProfile_Click(sender As Object, e As RoutedEventArgs) Handles cmdTerminateProfile.Click
        response.Text = "Terminating Profile..."
        Try
            Await Controller.TerminateProfile()
            response.Text = "Terminating Profile...OK"
        Catch ex As Exception
            response.Text = "Terminating Profile...Error:" & ex.ToString
        End Try


    End Sub

    Private Async Sub cmdGetInstanceListing_Click(sender As Object, e As RoutedEventArgs) Handles cmdGetInstanceListing.Click
        Try
            response.Text = "Getting instance Listing..."
            Dim profiles = Await Controller.GetProfileListing()
            Dim prflSelector = New ProfileSelector(profiles)
            prflSelector.ShowDialog()
            If prflSelector.isCanceled Then Return
            Dim profileID = prflSelector.SelectedProfile.Split(",")(0)
            prflSelector = Nothing

            Try
                Dim lst = Await Controller.GetInstanceListing(profileID)
                response.Text = ""
                For Each itm In lst
                    response.Text += itm.ToString & vbCrLf
                Next
            Catch ex As Exception
                response.Text = "Getting instance Listing...Error:" & ex.ToString
            End Try
        Catch ex As Exception
            response.Text = ex.ToString
            Return
        End Try

    End Sub

    Private Async Sub cmdDownloadInstance_Click(sender As Object, e As RoutedEventArgs) Handles cmdDownloadInstance.Click
        Try
            response.Text = "Downloading Instance Steps..."
            Dim profiles = Await Controller.GetProfileListing()
            Dim prflSelector = New ProfileSelector(profiles)
            prflSelector.ShowDialog()
            If prflSelector.isCanceled Then Return
            Dim profileID = prflSelector.SelectedProfile.Split(",")(0)

            prflSelector = Nothing

            Try
                Dim lst = Await Controller.GetInstanceListing(profileID)

                Dim selector As New InstanceSelector(lst)
                selector.ShowDialog()
                If selector.isCanceled Then Return

                Dim ret = Await Controller.DownloadInstance(profileID, selector.SelectedDate)
                response.Text = ""
                For Each itm In ret
                    response.Text += itm.ToString & vbCrLf
                Next

            Catch ex As Exception
                response.Text = "Downloading Instance Steps...Error:" & ex.ToString
                Return
            End Try
        Catch ex As Exception
            response.Text = ex.ToString
            Return
        End Try
    End Sub

    Private Async Sub cmdDownloadTrend_Click(sender As Object, e As RoutedEventArgs) Handles cmdDownloadTrend.Click
        Try
            response.Text = "Downloading Trend..."
            Dim profiles = Await Controller.GetProfileListing()
            Dim prflSelector = New ProfileSelector(profiles)
            prflSelector.ShowDialog()
            If prflSelector.isCanceled Then Return
            Dim profileID = prflSelector.SelectedProfile.Split(",")(0)

            prflSelector = Nothing

            Try
                Dim lst = Await Controller.GetInstanceListing(profileID)

                Dim selector As New InstanceSelector(lst)
                selector.ShowDialog()
                If selector.isCanceled Then Return

                Dim ret = Await Controller.DownloadTrendData(profileID, selector.SelectedDate)

                Dim fod = New SaveFileDialog
                fod.Filter = "Comma Seperated Values (*.csv)|*.csv"
                fod.AddExtension = False
                fod.ShowDialog()
                Dim idx As Integer = 0
                Using sw = New StreamWriter(fod.OpenFile())
                    sw.WriteLine("Idx, Probe0, Probe1, Output, Relay")
                    For Each itm In ret
                        sw.WriteLine(idx & "," & itm.ToString)
                        idx += 1
                    Next
                End Using
                response.Text = "Downloading Trend...OK"
                Process.Start(fod.FileName)
            Catch ex As Exception
                response.Text = "Downloading Trend...Error:" & ex.ToString
                Return
            End Try
        Catch ex As Exception
            response.Text = ex.ToString
            Return
        End Try

    End Sub

    Private Async Sub cmdDeleteEquipmentProfile_Click(sender As Object, e As RoutedEventArgs) Handles cmdDeleteEquipmentProfile.Click
        Try
            response.Text = "Deletng Equipment..."
            Dim EquipmentProfiles = Await Controller.GetEquipmentProfileListing()
            Dim prflSelector = New ProfileSelector(EquipmentProfiles)
            prflSelector.ShowDialog()
            If prflSelector.isCanceled Then Return
            Dim equipmentID = prflSelector.SelectedProfile.Split(",")(0)
            prflSelector = Nothing
            Try
                Await Controller.DeleteEquipmentProfile(equipmentID)
                response.Text = "Deletng Equipment...OK"
            Catch ex As Exception
                response.Text = "Deletng Equipment...Error:" & ex.ToString
            End Try
        Catch ex As Exception
            response.Text = ex.ToString
            Return
        End Try
    End Sub

    Private Async Sub cmdDeleteProfile_Click(sender As Object, e As RoutedEventArgs) Handles cmdDeleteProfile.Click
        Try
            response.Text = "Deleting Profile..."
            Dim profiles = Await Controller.GetProfileListing()
            Dim prflSelector = New ProfileSelector(profiles)
            prflSelector.ShowDialog()
            If prflSelector.isCanceled Then Return
            Dim profileID = prflSelector.SelectedProfile.Split(",")(0)
            prflSelector = Nothing
            Try
                Await Controller.DeleteProfile(profileID)
                response.Text = "Deleting Profile...OK"
            Catch ex As Exception
                response.Text = "Deleting Profile...Error:" & ex.ToString
            End Try
        Catch ex As Exception
            response.Text = ex.ToString
            Return
        End Try
    End Sub

    Private Async Sub cmdDeleteTrend_Click(sender As Object, e As RoutedEventArgs) Handles cmdDeleteTrend.Click
        Try
            response.Text = "Deleting Trend..."
            Dim profiles = Await Controller.GetProfileListing()
            Dim prflSelector = New ProfileSelector(profiles)
            prflSelector.ShowDialog()
            If prflSelector.isCanceled Then Return
            Dim profileID = prflSelector.SelectedProfile.Split(",")(0)
            prflSelector = Nothing
            Try
                Dim lst = Await Controller.GetInstanceListing(profileID)

                Dim selector As New InstanceSelector(lst)
                selector.ShowDialog()
                If selector.isCanceled Then Return

                Await Controller.DeleteInstance(profileID, selector.SelectedDate)
                response.Text = "Deleting Trend...OK"
            Catch ex As Exception
                response.Text = "Deleting Trend...Error:" & ex.ToString
            End Try
        Catch ex As Exception
            response.Text = ex.ToString
            Return
        End Try
    End Sub

    Private Async Sub cmdFirmware_Click(sender As Object, e As RoutedEventArgs) Handles cmdFirmware.Click
        Dim dlg As New Forms.FolderBrowserDialog
        dlg.SelectedPath = My.Settings.LastFirmwareFolder
        If dlg.ShowDialog = Forms.DialogResult.Cancel Then Return
        My.Settings.LastFirmwareFolder = dlg.SelectedPath
        My.Settings.Save()
        Try
            response.Text = "Uploading Firmware..."
            Try
                Using binFirmware = HEX_FirmwareProcessor.GenerateMasterBIN(dlg.SelectedPath)
                    Await Controller.uploadfirmware(binFirmware, Sub(prog As Single)
                                                                     Dispatcher.Invoke(Sub()
                                                                                           response.Text = "Uploading Firmware: " & (prog * 100).ToString("0.00") & "% Complete"
                                                                                       End Sub)
                                                                 End Sub)
                    response.Text = "Firmware Uploaded OK"
                End Using
            Catch ex As Exception
                response.Text = "Uploading Firmware...Error:" & ex.ToString
            End Try

        Catch ex As Exception

        End Try
    End Sub

    Private Async Sub cmdVersion_Click(sender As Object, e As RoutedEventArgs) Handles cmdVersion.Click
        response.Text = "DeviceInfo...."
        Try
            response.Text = Await Controller.GetVersion
        Catch ex As Exception
            response.Text = "DeviceInfo....Error:" & ex.ToString
        End Try


    End Sub

    Private Sub cmdTogglePolling_Click(sender As Object, e As RoutedEventArgs) Handles cmdTogglePolling.Click
        poll.Enabled = Not poll.Enabled
    End Sub

    Private Async Sub cmdUploadFile_Click(sender As Object, e As RoutedEventArgs) Handles cmdUploadFile.Click

        Dim dlg As New OpenFileDialog
        dlg.ShowDialog()

        response.Text = "Uploading File..."


        Try
            Using inp = dlg.OpenFile
                Dim fname = Path.GetFileName(dlg.FileName)
                Await Controller.uploadFile(inp, fname)
                response.Text = "File Uploaded! OK"
            End Using
        Catch ex As Exception
            response.Text = ex.ToString
        End Try
    End Sub

    Private Async Sub cmdSetTime_Click(sender As Object, e As RoutedEventArgs) Handles cmdSetTime.Click
        Await Controller.SetTime(Now)
    End Sub

    Private Async Sub cmdEditEquipmentProfile_Click(sender As Object, e As RoutedEventArgs) Handles cmdEditEquipmentProfile.Click
        Try
            response.Text = "Updating Profile..."
            Dim profiles = Await Controller.GetEquipmentProfileListing()
            Dim prflSelector = New ProfileSelector(profiles)
            prflSelector.ShowDialog()
            If prflSelector.isCanceled Then Return
            Dim EquipmentID = prflSelector.SelectedProfile.Split(",")(0)
            prflSelector = Nothing
            Try
                Dim temp = Await Controller.DownloadEquipmentProfile(EquipmentID)
                Dim edt As New EquipmentEditor
                edt.EquipmentID = EquipmentID
                edt.data = temp
                edt.ShowDialog()
                If edt.Canceled Then Return
                Await Controller.UploadEquipmentProfile(EquipmentID, edt.data)
                Await Controller.SetEquipmentProfile(EquipmentID)
                response.Text = "OK - Profile Updated"
            Catch ex As Exception
                response.Text = "Error:" & ex.ToString
            End Try
        Catch ex As Exception
            response.Text = ex.ToString
            Return
        End Try
    End Sub

    Private Async Sub cmdTruncate_Click(sender As Object, e As RoutedEventArgs) Handles cmdTruncate.Click
        response.Text = "Truncating..."
        Dim l As New List(Of PROFILE_STEP)
        l.Add(New PROFILE_STEP(18.3, 30, 120)) '3 Hours
        l.Add(New PROFILE_STEP(30, 30, 120)) '10 Minutes
        l.Add(New PROFILE_STEP(30, 15, 120)) '4 Hours
        l.Add(New PROFILE_STEP(15, 15, 120)) '5 Days

        Try
            Await Controller.TruncateProfile(l)
            response.Text = "Truncating...OK"
        Catch ex As Exception
            response.Text = "Truncating...Error:" & ex.ToString
        End Try

    End Sub

    Private Async Sub cmdUploadBlob_Click(sender As Object, e As RoutedEventArgs) Handles cmdUploadBlob.Click
        Dim dlg = New Forms.FolderBrowserDialog
        dlg.SelectedPath = My.Settings.LastBlob
        If dlg.ShowDialog() = Forms.DialogResult.Cancel Then Return
        My.Settings.LastBlob = dlg.SelectedPath
        My.Settings.Save()
        Dim blob = blobify.BlobThis(dlg.SelectedPath)
        Using fs As New FileStream("C:\blob.dat", FileMode.Create, FileAccess.ReadWrite)
            blob.Position = 0
            blob.CopyTo(fs)
        End Using
        blob.Position = 0
        Await Controller.uploadBlob(blob, Sub(prog As Single)
                                              Dispatcher.Invoke(Sub()
                                                                    response.Text = "Uploading Blob: " & (prog * 100).ToString("0.00") & "% Complete"
                                                                End Sub)
                                          End Sub)

        response.Text = "Blob Upload Completed OK"

    End Sub

    Private Async Sub cmdEraseBlob_Click(sender As Object, e As RoutedEventArgs) Handles cmdEraseBlob.Click
        response.Text = "Erasing Blob..."
        Await Controller.EraseBLOB
        response.Text = "Erasing Blob...OK"
    End Sub

    Private Sub Devices_SelectionChanged(sender As Object, e As SelectionChangedEventArgs) Handles Devices.SelectionChanged
        Dim selDev As TCON_Device = Devices.SelectedItem
        If IsNothing(selDev) = False Then
            Console.WriteLine(selDev.IP_Address)

        End If
    End Sub

    Private Async Sub cmdEraseFirmware_Click(sender As Object, e As RoutedEventArgs) Handles cmdEraseFirmware.Click
        response.Text = "Erasing Firmware..."
        Await Controller.EraseFirmware
        response.Text = "Erasing Firmware...OK"
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
                request.ServicePoint.ConnectionLeaseTimeout = 20000
                request.ServicePoint.UseNagleAlgorithm = False
                request.ServicePoint.Expect100Continue = False
                request.Timeout = 20000
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
                request.Abort()
                If retry = 3 Then
                    Throw tm
                Else
                    If IsNothing(request) = False Then request.Abort()
                    System.Threading.Thread.Sleep(100)
                End If
            Catch we As WebException
                request.Abort()
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
                request.Abort()
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
                request.ServicePoint.ConnectionLeaseTimeout = 20000
                request.ServicePoint.UseNagleAlgorithm = False
                request.ServicePoint.Expect100Continue = False
                request.Timeout = 20000
                request.Method = "PUT"
                request.ContentLength = 0

                st.Start()
                Dim T = request.GetResponseAsync()
                Dim X = Await Task.WhenAny(T, Task.Delay(500))
                If X.Equals(T) = False Then
                    request.Abort()
                    T.Dispose()
                    Continue For
                End If

                Using response As HttpWebResponse = T.Result
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
                request.Abort()
                If retry = 3 Then
                    If IsNothing(request) = False Then request.Abort()
                    System.Threading.Thread.Sleep(100)
                    Throw tm
                Else
                    If IsNothing(request) = False Then request.Abort()
                    System.Threading.Thread.Sleep(100)
                End If
            Catch we As WebException
                request.Abort()
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
                request.Abort()
                Debug.WriteLine(ex.ToString)
            End Try


        Next
        Return Nothing
    End Function
End Class
