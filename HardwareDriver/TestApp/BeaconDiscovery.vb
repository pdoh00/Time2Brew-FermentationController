Imports System.Net.Sockets
Imports System.Net
Imports System.IO

Public Class BeaconDiscovery
    Public Event OnDeviceDiscovered(DiscoveredDevices As Concurrent.ConcurrentBag(Of TCON_Device))

    Private mDiscoveredDevices As New Concurrent.ConcurrentBag(Of TCON_Device)
    Public ReadOnly Property DiscoveredDevices As Concurrent.ConcurrentBag(Of TCON_Device)
        Get
            Return mDiscoveredDevices
        End Get
    End Property

    Private NewDevices As New Concurrent.ConcurrentQueue(Of TCON_Device)

    Dim udpC As UdpClient
    Dim beaconPort As Integer
    Public Sub New(beaconPort As Integer)
        Me.beaconPort = beaconPort
        udpC = New UdpClient(beaconPort)
        udpC.BeginReceive(New AsyncCallback(AddressOf DataRX), udpC)
        'Dim dgram = Text.ASCIIEncoding.ASCII.GetBytes("TCON?")
        'udpC.Send(dgram, dgram.Length)
    End Sub

    Private Async Sub DataRX(ar As IAsyncResult)
        Dim c As UdpClient = ar.AsyncState
        Dim rxBytes = c.EndReceive(ar, New IPEndPoint(IPAddress.Any, Me.beaconPort))
        Dim token = Text.ASCIIEncoding.ASCII.GetString(rxBytes).Split(",")
        If token.Length = 3 Then
            Dim found As Boolean = False
            If token(0) = "TCON" Then
                For Each d In DiscoveredDevices
                    If d.IP_Address = token(2).Trim Then found = True
                Next
                If found = False Then
                    Dim queryResult = Await Comms_GET("http://" & token(2).Trim & "/api/deviceinfo")
                    If queryResult.StatusCode = 200 Then
                        Dim newD As New TCON_Device
                        newD.IP_Address = token(2).Trim
                        newD.MAC = token(1)

                        Dim BodyRsult = Text.ASCIIEncoding.ASCII.GetString(queryResult.Body).Split(vbCrLf)
                        newD.name = BodyRsult(0).Trim
                        newD.UUID = BodyRsult(1).Trim
                        DiscoveredDevices.Add(newD)
                        RaiseEvent OnDeviceDiscovered(DiscoveredDevices)
                    End If
                End If
            End If
        End If
        c.BeginReceive(New AsyncCallback(AddressOf DataRX), ar.AsyncState)
    End Sub

    Public Async Function Comms_GET(URL As String) As System.Threading.Tasks.Task(Of HTTP_Comms_Result)
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

End Class

Public Class TCON_Device
    Public name As String
    Public MAC As String
    Public IP_Address As String
    Public UUID As String
    Public Overrides Function ToString() As String
        Return name
    End Function
End Class