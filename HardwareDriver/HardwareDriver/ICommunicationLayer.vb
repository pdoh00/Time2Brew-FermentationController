Imports System.Threading.Tasks

Public Structure HTTP_Comms_Result
    Public StatusCode As Integer
    Public Body As Byte()

    Public Sub New(statusCode As Integer, Body As Byte())
        Me.StatusCode = statusCode
        Me.Body = Body
    End Sub
End Structure

Public Interface IHttpCommsProvider
    Function Comms_GET(URL As String) As Task(Of HTTP_Comms_Result)
    Function Comms_PUT(URL As String) As Task(Of HTTP_Comms_Result)
End Interface

