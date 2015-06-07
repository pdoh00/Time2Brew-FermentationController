Imports System.IO

Public Class blobify
    Shared Function BlobThis(srcFolder As String) As Stream
        Dim blobNameBytes(255) As Byte
        Dim CheckSum As UInt16
        Dim ret As New MemoryStream
        For Each File In Directory.GetFiles(srcFolder, "*.*", SearchOption.AllDirectories)
            Console.WriteLine(File)
            Dim Blob = blobifyThisFile(File, CheckSum)

            'Write the file name
            Dim blobName = Text.ASCIIEncoding.ASCII.GetBytes(File.Replace(srcFolder, "").Replace("\", "/"))
            For x = 0 To 255
                blobNameBytes(x) = 0
            Next
            Array.Copy(blobName, blobNameBytes, blobName.Length)
            ret.Write(blobNameBytes, 0, 256)

            'Write the length
            Dim blobLength As UInt32 = Blob.Length
            ret.Write(BitConverter.GetBytes(blobLength), 0, 4)

            'Write the CheckSum
            ret.Write(BitConverter.GetBytes(CheckSum), 0, 2)

            Blob.Position = 0
            Blob.CopyTo(ret)
        Next
        Return ret
    End Function

    Private Shared Function blobifyThisFile(src As String, ByRef CheckSum As UInt16) As Stream
        Dim extension = Path.GetExtension(src)
        Dim ret As New MemoryStream
        Dim data() As Byte
        Dim buff() As Byte
        Using inFile As New FileStream(src, FileMode.Open, FileAccess.Read)
            Using outFile As New MemoryStream()
                Using compressor As New IO.Compression.GZipStream(outFile, Compression.CompressionMode.Compress)
                    ReDim buff(inFile.Length - 1)
                    inFile.Read(buff, 0, buff.Length)
                    compressor.Write(buff, 0, buff.Length)
                    compressor.Close()
                    data = outFile.ToArray()
                End Using
            End Using
        End Using

        CheckSum = FletcherChecksum.Fletcher16(data, 0, data.Length)
        Dim contentType As String
        Select Case extension.ToLower
            Case ".htm", ".html"
                contentType = "text/html"
            Case ".jpeg", ".jpg"
                contentType = "image/jpeg"
            Case ".gif"
                contentType = "image/gif"
            Case ".tif", ".tiff"
                contentType = "image/tiff"
            Case ".css"
                contentType = "text/css"
            Case ".js"
                contentType = "text/javascript"
            Case ".txt"
                contentType = "text/plain"
            Case ".png"
                contentType = "image/png"
            Case Else
                contentType = "application/octext-stream"
        End Select

        AddString(ret, String.Format(
                  "HTTP/1.1 200 OK" & vbCrLf & _
                  "Connection: Keep-Alive" & vbCrLf & _
                  "Cache-Control: 600" & vbCrLf & _
                  "ETag: ""{0}""" & vbCrLf & _
                  "Content-Type: {1}" & vbCrLf & _
                  "Content-Encoding: gzip" & vbCrLf & _
                  "Content-Length: {2}" & vbCrLf & _
                  vbCrLf, Hex(checksum), contentType, data.Length.ToString))
        ret.Write(data, 0, data.Length)
        Return ret
    End Function

    Private Shared Sub AddString(dst As Stream, msg As String)
        Dim buff = Text.ASCIIEncoding.ASCII.GetBytes(msg)
        dst.Write(buff, 0, buff.Length)
    End Sub

End Class
