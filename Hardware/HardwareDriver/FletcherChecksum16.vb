Public Class FletcherChecksum
    Shared Function Fletcher16(data() As Byte, offset As Integer, bCount As Integer) As UInt16
        Dim sum1 As UInt16 = &HFF
        Dim sum2 As UInt16 = &HFF
        Dim idx As Integer = offset
        While (bCount)
            Dim tlen = Math.Min(20, bCount)
            bCount -= tlen
            While (tlen)
                sum1 += data(idx)
                sum2 += sum1
                idx += 1
                tlen -= 1
            End While
            sum1 = (sum1 And &HFF) + (sum1 >> 8)
            sum2 = (sum2 And &HFF) + (sum2 >> 8)
        End While

        sum1 = (sum1 And &HFF) + (sum1 >> 8)
        sum2 = (sum2 And &HFF) + (sum2 >> 8)

        Return (sum2 << 8) Or sum1

    End Function

End Class
