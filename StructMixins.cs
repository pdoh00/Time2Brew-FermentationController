using System;
using System.IO;

namespace FermentationController
{
	public static class StructMixins
	{
//		public static T ReadStruct<T>(Stream data)
//		{
//			var size = sizeof(T);
//			var buffer = new byte[size];
//			data.Read (buffer, 0, size);
//			var handle = GCHandle
//		}
	}

	public static class StreamMixins
	{
		public static UInt16 ReadUInt16(this Stream s)
		{
			var buffer = new byte[2];
			return (UInt16)s.Read(buffer, 0, 2);
		}

		public static UInt32 ReadUInt32(this Stream s)
		{
			var buffer = new byte[4];
			return (UInt32)s.Read(buffer, 0, 4);
		}

		public static string ReadString(this Stream s, int length)
		{
			var buffer = new byte[length];
			s.Read (buffer, 0, length);
			return System.Text.Encoding.UTF8.GetString(buffer, 0, length);
		}
	}
}


