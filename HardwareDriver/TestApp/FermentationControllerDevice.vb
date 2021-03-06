﻿Imports System.Threading.Tasks
Imports System.IO


Public Class FermentationControllerDevice
    Private ReadOnly EPOCH As New DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc)

    Private RootAddress As String
    Private COM As IHttpCommsProvider
    Public Sub New(COM As IHttpCommsProvider, DeviceAddress As String)
        Me.RootAddress = DeviceAddress
        Me.COM = COM
    End Sub

    Public Sub ChangeRootAddress(newRoot As String)
        Me.RootAddress = newRoot
    End Sub

    ''' <summary>
    ''' Returns the STATUS of the controller.  Can be called any time. Does not require authentication.
    ''' </summary>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public Async Function GetStatus() As Task(Of CONTROLLER_STATUS)
        Dim Result As HTTP_Comms_Result = Await COM.Comms_GET(RootAddress & "/api/status")
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
        Return New CONTROLLER_STATUS(Result.Body)
    End Function


    ''' <summary>
    ''' Returns the RTC Time on the controller. Can be called any time. Does not require authentication.
    ''' </summary>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public Async Function GetTime() As Task(Of DateTime)
        Dim Result As HTTP_Comms_Result = Await COM.Comms_GET(RootAddress & "/api/time")
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
        Dim timeOffset As UInt32 = BitConverter.ToUInt32(Result.Body, 0)
        Return EPOCH.AddSeconds(timeOffset)
    End Function

    ''' <summary>
    ''' Sets the RTC time on the controller to the provided value. Can be called any time. Requires Authentication.
    ''' </summary>
    ''' <param name="NewTime"></param>
    ''' <remarks></remarks>
    Public Async Function SetTime(NewTime As DateTime) As Task
        Dim timeOffset As UInt32 = (NewTime.ToUniversalTime - EPOCH).TotalSeconds
        Dim Result As HTTP_Comms_Result = Await COM.Comms_PUT(RootAddress & "/api/time?time=" & timeOffset.ToString)
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
    End Function

    ''' <summary>
    ''' ECHO test. Will return whatever data is provided. Can be called any time. Does not require authentication.
    ''' </summary>
    ''' <param name="msg"></param>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public Async Function EchoGet(msg As String) As Task(Of String)
        Dim Result As HTTP_Comms_Result = Await COM.Comms_GET(RootAddress & "/api/echo?echo=" & msg)
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
        Return Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Length)
    End Function

    ''' <summary>
    ''' ECHO test. Will return whatever data is provided. Can be called any time. Does not require authentication.
    ''' </summary>
    ''' <param name="msg"></param>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public Async Function EchoPut(msg As String) As Task(Of String)
        Dim Result As HTTP_Comms_Result = Await COM.Comms_PUT(RootAddress & "/api/echo?echo=" & msg)
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
        Return Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Length)
    End Function

    ''' <summary>
    ''' Returns a listing of all profiles which are on the controller. Can be called any time. Does not require authentication.
    ''' </summary>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public Async Function GetProfileListing() As Task(Of List(Of String))
        Dim ret As New List(Of String)
        Dim Result As HTTP_Comms_Result = Await COM.Comms_GET(RootAddress & "/api/profile")
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
        Using ms As New MemoryStream(Result.Body)
            Using rdr As New StreamReader(ms, Text.UTF8Encoding.UTF8)
                While (rdr.EndOfStream = False)
                    ret.Add(rdr.ReadLine)
                End While
            End Using
        End Using
        Return ret
    End Function

    ''' <summary>
    ''' Downloads the given profile from the controller.  The profile must exist otherwise an exception will be thrown.
    ''' For a list of profiles see the GetProfileListing command. Can be called any time. Does not require authentication.
    ''' </summary>
    ''' <param name="profileID"></param>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public Async Function DownloadProfile(profileID As String) As Task(Of TEMPERATURE_PROFILE)
        Dim Result As HTTP_Comms_Result = Await COM.Comms_GET(RootAddress & "/api/profile?id=" & profileID)
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
        Dim ret As New TEMPERATURE_PROFILE(Result.Body)
        Return ret
    End Function


    ''' <summary>
    ''' Uploads a temperature profile to the controller.  If the profile already exists it will be overwritten without warning.
    ''' Can be called any time. Requires Authentication.
    ''' </summary>
    ''' <param name="profileID"></param>
    ''' <param name="profileData"></param>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public Async Function UploadProfile(profileID As String, profileData As TEMPERATURE_PROFILE) As Task(Of Boolean)
        Dim FileOffset As UInt32 = 0
        Dim readBuffer(511) As Byte
        Dim firstChunk As Boolean = True
        Dim url As String
        Using ms As New MemoryStream(profileData.Serialize)
            ms.Position = 0
            While (ms.Position < ms.Length)
                Dim bytesRead = ms.Read(readBuffer, 0, 512)
                Dim sendData(bytesRead - 1) As Byte
                Array.Copy(readBuffer, sendData, bytesRead)
                Dim chksum = FletcherChecksum.Fletcher16(sendData, 0, sendData.Length)

                If (firstChunk) Then
                    If (profileID = "") Then
                        url = RootAddress & "/api/profile?chksum=" & chksum
                    Else
                        url = RootAddress & "/api/profile?id=" & profileID & "chksum=" & chksum
                    End If
                    firstChunk = False
                Else
                    url = RootAddress & "/api/profile?id=" & profileID & "chksum=" & chksum & "&offset=" & FileOffset
                End If
                url += "&content=" & ToURL_Safe_base64String(sendData)
                Dim Result As HTTP_Comms_Result = Await COM.Comms_PUT(url)
                If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
                profileID = Text.ASCIIEncoding.ASCII.GetString(Result.Body)
                FileOffset += bytesRead
            End While
        End Using
        Return True
    End Function

    ''' <summary>
    ''' Starts the given profile.  The controller must be in manual mode before executing this command otherwise an exception will be thrown.
    ''' Requires authentication.
    ''' </summary>
    ''' <param name="profileID"></param>
    ''' <remarks></remarks>
    Public Async Function ExecuteProfile(profileID As String) As Task
        Dim Result As HTTP_Comms_Result = Await COM.Comms_PUT(RootAddress & "/api/executeprofile?id=" & profileID)
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
    End Function

    Public Async Function DeleteProfile(profileID As String) As Task
        Dim Result As HTTP_Comms_Result = Await COM.Comms_PUT(RootAddress & "/api/deleteprofile?id=" & profileID)
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
    End Function

    ''' <summary>
    ''' Stops execution of the currently running profile and returns the controller to manual setpoint mode.
    ''' Requires authentication.
    ''' </summary>
    ''' <remarks></remarks>
    Public Async Function TerminateProfile() As Task
        Dim Result As HTTP_Comms_Result = Await COM.Comms_PUT(RootAddress & "/api/terminateprofile")
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
    End Function

    ''' <summary>
    ''' This will stop the currently running profile, truncate any remaining steps, and then append the provided steps.  A profile must be executing otherwise an exception will be thrown.
    ''' Requires authentication.
    ''' </summary>
    ''' <param name="profileSteps">A list of profile steps that should be appened to the currently running profile after trucnation.</param>
    ''' <remarks></remarks>
    Public Async Function TruncateProfile(profileSteps As List(Of PROFILE_STEP)) As Task
        Using ms As New MemoryStream()
            For Each prflStep In profileSteps
                ms.Write(prflStep.Serialize, 0, 8)
            Next
            Dim sendData = ms.ToArray
            Dim chksum = FletcherChecksum.Fletcher16(sendData, 0, sendData.Length)
            Dim url As String = RootAddress & "/api/truncateprofile?chksum=" & chksum
            url += "&content=" & ToURL_Safe_base64String(sendData)
            Dim Result As HTTP_Comms_Result = Await COM.Comms_PUT(url)
            If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
        End Using
    End Function

    ''' <summary>
    ''' Returns a listing of instances of the given profile name.
    ''' For a list of profiles see the GetProfileListing command.
    ''' Can be called anytime. Does not require authentication.
    ''' </summary>
    ''' <param name="profileID"></param>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public Async Function GetInstanceListing(profileID As String) As Task(Of List(Of DateTime))
        Dim ret As New List(Of DateTime)
        Dim Result As HTTP_Comms_Result = Await COM.Comms_GET(RootAddress & "/api/instancesteps?id=" & profileID)
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
        Using ms As New MemoryStream(Result.Body)
            Using rdr As New StreamReader(ms, Text.UTF8Encoding.UTF8)
                While (rdr.EndOfStream = False)
                    Dim token = rdr.ReadLine
                    Dim timeOffset As UInt32
                    If UInt32.TryParse(token, timeOffset) Then
                        ret.Add(EPOCH.AddSeconds(timeOffset).ToLocalTime)
                    End If
                End While
            End Using
        End Using
        Return ret
    End Function

    ''' <summary>
    ''' Downloads the actual profile that was executed for the given profile and given instance of that profile. This will reflect any truncations or early terminations of the profile.
    ''' For a list of profiles see the GetProfileListing command.
    ''' For a list of instances of a profile see the GetInstanceListing command
    ''' Can be called anytime. Does not require authentication.
    ''' </summary>
    ''' <param name="profileID"></param>
    ''' <param name="Instance"></param>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public Async Function DownloadInstance(profileID As String, Instance As DateTime) As Task(Of List(Of PROFILE_STEP))
        Dim timeOffset = (Instance.ToUniversalTime - EPOCH).TotalSeconds
        Dim Result As HTTP_Comms_Result = Await COM.Comms_GET(RootAddress & "/api/instancesteps?id=" & profileID & "&instance=" & timeOffset)
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
        Dim ret As New List(Of PROFILE_STEP)
        Dim offset As Integer = 0
        While (Result.Body.Length - offset >= 12)
            ret.Add(New PROFILE_STEP(Result.Body, offset))
            offset += 12
        End While
        Return ret
    End Function

    Public Async Function DeleteInstance(profileID As String, Instance As DateTime) As Task
        Dim timeOffset = (Instance.ToUniversalTime - EPOCH).TotalSeconds
        Dim Result As HTTP_Comms_Result = Await COM.Comms_PUT(RootAddress & "/api/deleteinstance?id=" & profileID & "&instance=" & timeOffset)
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
    End Function

    ''' <summary>
    ''' Downloads the temperare trend for a given instance of a given profile.  Both the profile name and instance must be provided.  
    ''' For a list of profiles see the GetProfileListing command.
    ''' For a list of instances of a profile see the GetInstanceListing command
    ''' Can be called anytime. Does not require authentication.
    ''' </summary>
    ''' <param name="profileID"></param>
    ''' <param name="Instance"></param>
    ''' <returns>A list of trend records originating from Instance Date/Time, one sample every minute</returns>
    ''' <remarks></remarks>
    Public Async Function DownloadTrendData(profileID As String, Instance As DateTime) As Task(Of List(Of TREND_RECORD))
        Dim timeOffset = (Instance.ToUniversalTime - EPOCH).TotalSeconds
        Dim Result As HTTP_Comms_Result = Await COM.Comms_GET(RootAddress & "/api/instancetrend?id=" & profileID & "&instance=" & timeOffset)
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
        Using fs As New FileStream("C:\rawProfile.dat", FileMode.Create, FileAccess.Write)
            fs.Write(Result.Body, 0, Result.Body.Length)
        End Using
        Dim ret As New List(Of TREND_RECORD)
        Dim offset As Integer = 0
        While (Result.Body.Length - offset >= 4)
            ret.Add(New TREND_RECORD(Result.Body, offset))
            offset += 4
        End While
        Return ret
    End Function

    ''' <summary>
    ''' Gets the current temperature of either temperature probe.
    ''' Can be called anytime. Does not require authentication.
    ''' </summary>
    ''' <param name="ProbeID">0 or 1</param>
    ''' <returns>The probe temperature in degrees C</returns>
    ''' <remarks></remarks>
    Public Async Function GetTemperature(ProbeID As Integer) As Task(Of Double)
        Dim Result As HTTP_Comms_Result = Await COM.Comms_GET(RootAddress & "/api/temperature?probe=" & ProbeID)
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
        Return Double.Parse(Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Length)) * 0.01
    End Function

    ''' <summary>
    ''' Sets the manual mode temperature setpoint.  Only possible if the controller is not running a temperature profile otherwise an exception will be thrown.
    ''' </summary>
    ''' <param name="SetPoint_C">Setpoint in degrees C</param>
    ''' <remarks></remarks>
    Public Async Function SetManualMode(SetPoint_C As Double) As Task
        Dim setpoint As Int16 = SetPoint_C * 10
        Dim Result As HTTP_Comms_Result = Await COM.Comms_PUT(RootAddress & "/api/temperature?setpoint=" & setpoint)
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
    End Function

    ''' <summary>
    ''' Returns a list of equipment profile names.
    ''' Can be called anytime. Does not require authentication.
    ''' </summary>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public Async Function GetEquipmentProfileListing() As Task(Of List(Of String))
        Dim ret As New List(Of String)
        Dim Result As HTTP_Comms_Result = Await COM.Comms_GET(RootAddress & "/api/equipmentprofile")
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
        Using ms As New MemoryStream(Result.Body)
            Using rdr As New StreamReader(ms, Text.UTF8Encoding.UTF8)
                While (rdr.EndOfStream = False)
                    ret.Add(rdr.ReadLine)
                End While
            End Using
        End Using
        Return ret
    End Function

    ''' <summary>
    ''' Downloads and equipment profile from the controller.  For a listing of equipment profiles see the GetEquipmentProfileListing command.
    ''' If the profile does not exist a FermCtrlCommsException will be thrown.
    ''' Can be called anytime. Does not require authentication.
    ''' </summary>
    ''' <param name="equipmentID">Up to 63 characters are allowed.</param>
    ''' <returns></returns>
    ''' <remarks></remarks>
    Public Async Function DownloadEquipmentProfile(equipmentID As String) As Task(Of EQUIPMENT_PROFILE)
        Dim Result As HTTP_Comms_Result = Await COM.Comms_GET(RootAddress & "/api/equipmentprofile?id=" & equipmentID)
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
        Dim ret As New EQUIPMENT_PROFILE(Result.Body)
        Return ret
    End Function

    ''' <summary>
    ''' Uploads an equipment profile to the controller.  If the profile already exists it will be overwritten without warning.
    ''' Can be called anytime. Requires authentication.
    ''' </summary>
    ''' <param name="equipmentProfileID">Up to 63 characters are allowed.</param>
    ''' <param name="equipmentProfile"></param>
    ''' <remarks></remarks>
    Public Async Function UploadEquipmentProfile(equipmentProfileID As String, equipmentProfile As EQUIPMENT_PROFILE) As Task
        Dim sendData = equipmentProfile.Serialize()
        Dim chksum = FletcherChecksum.Fletcher16(sendData, 0, sendData.Length)
        Dim url As String
        If equipmentProfileID = "" Then
            url = RootAddress & "/api/equipmentprofile?chksum=" & chksum
            url += "&content=" & ToURL_Safe_base64String(sendData)
        Else
            url = RootAddress & "/api/equipmentprofile?id=" & equipmentProfileID
            url += "&chksum=" & chksum
            url += "&content=" & ToURL_Safe_base64String(sendData)
        End If

        Dim Result As HTTP_Comms_Result = Await COM.Comms_PUT(url)
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
    End Function

    ''' <summary>
    ''' Makes the named equipment profile active.  Any running or future temperature profiles, or manual mode, will run under this equipment profile after this command.
    ''' If the profile does not exist a FermCtrlCommsException will be thrown.
    ''' Can be called anytime. Requires authentication.
    ''' </summary>
    ''' <param name="equipmentProfileID">Name of the equipment profile.  For a listing of profiles see GetEquipmentProfileListing command</param>
    ''' <remarks></remarks>
    Public Async Function SetEquipmentProfile(equipmentProfileID As String) As Task
        Dim Result As HTTP_Comms_Result = Await COM.Comms_PUT(RootAddress & "/api/setequipmentprofile?id=" & equipmentProfileID)
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
    End Function

    Public Async Function DeleteEquipmentProfile(equipmentProfileID As String) As Task
        Dim Result As HTTP_Comms_Result = Await COM.Comms_PUT(RootAddress & "/api/deleteequipmentprofile?id=" & equipmentProfileID)
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
    End Function

    ''' <summary>
    ''' A very big deal!  This will format the internal storage and reset everything back to the factory default state...
    ''' This command is only possible when the controller is in 'Configuration Mode'.  See the 'RebootInConfigMode' command.
    ''' </summary>
    ''' <remarks></remarks>
    Public Async Function WipeAndFactoryDefault() As Task
        Dim Result As HTTP_Comms_Result = Await COM.Comms_PUT(RootAddress & "/api/format?confirm=format")
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
    End Function


    ''' <summary>
    ''' Updates the security username and password which is requried for access to restricted commands.  
    ''' This command is only possible when the controller is in 'Configuration Mode'.  See the 'RebootInConfigMode' command.
    ''' </summary>
    ''' <param name="Username">Up to 63 characters are allowed.</param>
    ''' <param name="Password">Up to 63 characters are allowed.</param>
    ''' <remarks></remarks>
    Public Async Function UpdateSecurityCredentials(Username As String, Password As String) As Task
        Dim Result As HTTP_Comms_Result = Await COM.Comms_PUT(RootAddress & "/api/updatecredentials?username=" & Username & "&password=" & Password)
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
    End Function

    ''' <summary>
    ''' Reboot the fermentation controller.  Upon restart the controller will attempt to recover any running activity.
    ''' Can be called anytime. Requires authentication.
    ''' </summary>
    ''' <remarks></remarks>
    Public Async Function Reboot() As Task
        Dim Result As HTTP_Comms_Result = Await COM.Comms_PUT(RootAddress & "/api/restart?confirm=restart")
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
    End Function


    ''' <summary>
    ''' This will update the WiFi configuration.  This command is only possible when the controller is in 'Configuration Mode'.  See the 'RebootInConfigMode' command.
    ''' </summary>
    ''' <param name="opMode">The Operating Mode for the controller.  STATION means it will attemp to join an existing wireless network.  SOFT_AP will create its own network.</param>
    ''' <param name="channel">WiFi Channel to transmit on. Only applicable when opMode=SOFT_AP</param>
    ''' <param name="encryption">WiFi Encryption Mode. Only applicable when opMode=SOFT_AP. When acting as a station it will determine the encryption used from the wireless AP.</param>
    ''' <param name="deviceName">This is the name of the fermentation controller on the network and impacts the mDNS name as well as what name appears in Network Devices when browsing the SSDP.
    ''' Up to 63 characters are allowed.</param>
    ''' <param name="Wifi_Password">In SOFT_AP mode this sets the network password that any clients will have to know.  
    ''' In STATION mode this is the password to the wireless network the controller will be trying to join.
    ''' Up to 31 characters are allowed.</param>
    ''' <param name="Wifi_SSID">In SOFT_AP mode this sets the name of the wireless network that clients will connect to.  
    ''' In STATION mode this sets the name of the wireless network that the controller will try to join.
    ''' Up to 31 characters are allowed.</param>
    ''' <remarks></remarks>
    Public Async Function UpdateWifiConfig(opMode As WIFI_OP_MODES, channel As Integer, encryption As WIFI_ENCRYPTION_MODES, deviceName As String, Wifi_Password As String, Wifi_SSID As String) As Task
        Dim url = String.Format("{0}/api/wificonfig?mode={1}&channel={2}&encryption={3}&name={4}&password={5}&ssid={6}",
                                RootAddress, opMode, channel, encryption, deviceName, Wifi_Password, Wifi_SSID)
        Dim Result As HTTP_Comms_Result = Await COM.Comms_PUT(url)
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
    End Function

    Public Async Function TrimFileSystem() As Task
        Try
            Dim Result As HTTP_Comms_Result = Await COM.Comms_PUT(RootAddress & "/api/trimfilesystem")
            If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
        Catch ex As Exception

        End Try
    End Function

    Public Async Function EraseBLOB() As Task
        Try
            Dim Result As HTTP_Comms_Result = Await COM.Comms_PUT(RootAddress & "/api/eraseblob?wipe=confirm")
            If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
        Catch ex As Exception

        End Try
    End Function


    Public Async Function EraseFirmware() As Task
        Try
            Dim Result As HTTP_Comms_Result = Await COM.Comms_PUT(RootAddress & "/api/erasefirmware?wipe=confirm")
            If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
        Catch ex As Exception

        End Try
    End Function

    Public Async Function uploadfirmware(blobFile As Stream, prog As Action(Of Single)) As Task
        Dim first As Boolean = True
        Dim buffer(512) As Byte
        blobFile.Position = 0
        While blobFile.Position < blobFile.Length - 1
            prog(blobFile.Position / blobFile.Length)
            Dim offset = blobFile.Position
            Dim bytesToSend = Math.Min(blobFile.Length - blobFile.Position, 512)
            ReDim buffer(bytesToSend - 1)
            blobFile.Read(buffer, 0, bytesToSend)
            While (1)
                Dim res = Await SendFirmwareChunk(buffer, offset, bytesToSend)
                If res = 1 Then Exit While
            End While
        End While
        Debug.Print("DONE")
    End Function

    Private Async Function SendFirmwareChunk(Buffer As Byte(), offset As Integer, bCount As Integer) As Task(Of Integer)
        Try
            Dim chksum As UInt16 = FletcherChecksum.Fletcher16(Buffer, 0, bCount)
            Dim url As String = RootAddress & "/api/uploadfirmware?offset=" & offset
            url += "&chksum=" & chksum
            url += "&content=" & ToURL_Safe_base64String(Buffer, 0, bCount)
            Dim Result As HTTP_Comms_Result = Await COM.Comms_PUT(url)
            If Result.StatusCode <> 200 Then Return 0
            If Result.Body.Length < bCount Then Return 0
            For idx = 0 To bCount - 1
                If Buffer(idx) <> Result.Body(idx) Then
                    Console.WriteLine("Mismatch @ " & idx)
                    Return -1
                End If
            Next

            Return 1
        Catch ex As Exception
            Return 0
        End Try
    End Function


    Public Async Function uploadFile(inputFile As Stream, fname As String) As Task
        Dim first As Boolean = True
        Dim isLast As Boolean = False
        Dim buffer(512) As Byte
        While inputFile.Position < inputFile.Length - 1
            Dim offset = inputFile.Position
            Dim bytesToSend = Math.Min(inputFile.Length - inputFile.Position, 512)
            If bytesToSend < 512 OrElse inputFile.Position = inputFile.Length Then isLast = True Else isLast = False
            ReDim buffer(bytesToSend - 1)
            inputFile.Read(buffer, 0, bytesToSend)
            Dim chksum As UInt16 = FletcherChecksum.Fletcher16(buffer, 0, bytesToSend)
            Dim url As String = RootAddress & "/api/uploadfile?fname=" & fname & "&offset=" & offset
            If first Then
                url += "&overwrite=y"
                first = False
            Else
                url += "&overwrite=n"
            End If

            If isLast Then
                url += "&finalize=y"
            Else
                url += "&finalize=n"
            End If

            url += "&chksum=" & chksum
            url += "&content=" & ToURL_Safe_base64String(buffer, 0, bytesToSend)
            While (1)
                Try
                    Console.Write("Push...")
                    Dim Result As HTTP_Comms_Result = Await COM.Comms_PUT(url)

                    If Result.StatusCode <> 200 Then
                        Console.WriteLine(Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
                        Continue While ' Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
                    Else
                        Console.WriteLine("OK")
                    End If
                    Exit While
                Catch ex As Exception

                End Try
            End While
        End While
    End Function

    Public Async Function uploadBlob(blobFile As Stream, prog As Action(Of Single)) As Task
        Dim first As Boolean = True
        Dim buffer(512) As Byte
        blobFile.Position = 0
        While blobFile.Position < blobFile.Length - 1
            prog(blobFile.Position / blobFile.Length)
            Dim offset = blobFile.Position
            Dim bytesToSend = Math.Min(blobFile.Length - blobFile.Position, 512)
            ReDim buffer(bytesToSend - 1)
            blobFile.Read(buffer, 0, bytesToSend)
            While (1)
                Dim res = Await SendBlobChunk(buffer, offset, bytesToSend)
                If res = 1 Then Exit While
            End While
        End While
        Debug.Print("DONE")
    End Function

    Private Async Function SendBlobChunk(Buffer As Byte(), offset As Integer, bCount As Integer) As Task(Of Integer)
        Try
            Dim chksum As UInt16 = FletcherChecksum.Fletcher16(Buffer, 0, bCount)
            Dim url As String = RootAddress & "/api/uploadblob?offset=" & offset
            url += "&chksum=" & chksum
            url += "&content=" & ToURL_Safe_base64String(Buffer, 0, bCount)
            Dim Result As HTTP_Comms_Result = Await COM.Comms_PUT(url)
            If Result.StatusCode <> 200 Then Return 0
            If Result.Body.Length < bCount Then Return 0
            For idx = 0 To bCount - 1
                If Buffer(idx) <> Result.Body(idx) Then
                    Console.WriteLine("Mismatch @ " & idx)
                    Return -1
                End If
            Next

            Return 1
        Catch ex As Exception
            Return 0
        End Try
    End Function

    Public Async Function GetVersion() As Task(Of String)
        Dim Result As HTTP_Comms_Result = Await COM.Comms_GET(RootAddress & "/api/deviceinfo")
        If Result.StatusCode <> 200 Then Throw New FermCtrlCommsException(Result.StatusCode, Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count))
        Return Text.UTF8Encoding.UTF8.GetString(Result.Body, 0, Result.Body.Count)
    End Function

    Private Function ToURL_Safe_base64String(data As Byte(), offset As Integer, len As Integer) As String
        Return Convert.ToBase64String(data, offset, len).Replace("+", "%2B")
    End Function

    Private Function ToURL_Safe_base64String(data As Byte()) As String
        Return Convert.ToBase64String(data).Replace("+", "%2B")
    End Function


End Class

Public Class FermCtrlCommsException
    Inherits Exception

    Public Sub New(StatusCode As Integer, StatusMessage As String)
        MyBase.New(StatusCode.ToString & ":" & StatusMessage)
    End Sub
End Class