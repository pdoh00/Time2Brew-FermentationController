Imports System.IO

Public Structure TEMPERATURE_PROFILE
    Public ProfileName As String
    Public Steps As List(Of PROFILE_STEP)

    Public Function Serialize() As Byte()
        Dim bb(63) As Byte
        For x = 0 To 63
            bb(x) = 0
        Next
        Dim nameBuff() As Byte = Text.ASCIIEncoding.ASCII.GetBytes(Me.ProfileName)
        Array.Copy(nameBuff, 0, bb, 0, nameBuff.Length)

        Using ms As New MemoryStream()
            ms.Write(bb, 0, 64)
            For Each stp In Steps
                ms.Write(stp.Serialize, 0, 8)
            Next
            Return ms.ToArray()
        End Using
    End Function

    Public Sub New(sourceData As Byte())
        Steps = New List(Of PROFILE_STEP)

        Dim offset As Integer = 0
        ProfileName = GetString(sourceData, 0, 64)
        offset = 64

        While (sourceData.Length - offset >= 8)
            Dim stp = New PROFILE_STEP(sourceData, offset)
            Steps.Add(stp)
            offset += 8
        End While
    End Sub

    Public Sub New(ProfileName As String, L As List(Of PROFILE_STEP))
        Me.ProfileName = ProfileName
        Me.Steps = New List(Of PROFILE_STEP)
        Me.Steps.AddRange(L)
    End Sub

    Public Overrides Function ToString() As String
        Dim ret As String = "Name: " & Me.ProfileName & vbCrLf
        For Each stp In Steps
            ret += "  " & stp.ToString() & vbCrLf
        Next
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

