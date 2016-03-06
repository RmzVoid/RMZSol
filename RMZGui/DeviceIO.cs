using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using Microsoft.Win32.SafeHandles;

namespace RMZGui
{
	// Reads data from driver
	public class DeviceIO
	{
		//
		// size of buffer for read and write
		private const int bufferSize = 16384;
		private byte[] buffer = new byte[bufferSize];

		//
		// stream for data exchange with device
		private FileStream deviceStream = null;

		//
		// token for cancelling
		private CancellationToken cancelToken;

		//
		// dispatch incoming packets
		private PacketDispatcher dispatcher;

		public DeviceIO(string devicePath, CancellationToken cancel, PacketDispatcher dispatcher)
		{
			//
			// Open device
			SafeFileHandle deviceHandle = Win32.CreateFile(devicePath,
				Win32.FileAccess.GenericRead | Win32.FileAccess.GenericWrite,
				Win32.FileShare.None,
				IntPtr.Zero,
				Win32.CreationDisposition.OpenExisting,
				Win32.FileAttributes.Normal | Win32.FileAttributes.Overlapped,
				IntPtr.Zero);

			if (deviceHandle.IsInvalid)
				Marshal.ThrowExceptionForHR(Marshal.GetHRForLastWin32Error());

			//
			// Open stream for data flow
			deviceStream = new FileStream(deviceHandle, FileAccess.ReadWrite, bufferSize, true);

			cancelToken = cancel;
			this.dispatcher = dispatcher;

			// Start read
			Task.Run(new Action(Read));
		}

		private void Read()
		{
			while (!cancelToken.IsCancellationRequested)
			{
				//
				// need to know how to interrupt on demand this fucking Read
				// Close, Dispose not helped, interrupting Task not recommended
				int bytesReaded = deviceStream.Read(buffer, 0, bufferSize);

				if (bytesReaded > 0)
				{
					BinaryReader br = new BinaryReader(new MemoryStream(buffer, 0, bytesReaded));

					Packet packet = new Packet()
					{
						flowId = br.ReadUInt64(),
						direction = (Source)br.ReadUInt32(),
						length = br.ReadInt32()
					};

					packet.data = br.ReadBytes(packet.length);

					dispatcher.DispatchPacket(this, packet);
				}
			}
		}

		public void Write(Packet packet)
		{
			using (MemoryStream ms = new MemoryStream())
			{
				using (BinaryWriter bw = new BinaryWriter(ms))
				{
					bw.Write(packet.flowId);
					bw.Write((uint)packet.direction);
					bw.Write(packet.length);
					bw.Write(packet.data);

					deviceStream.Write(ms.ToArray(), 0, (int)ms.Length);
					deviceStream.Flush();
				}
			}
		}
	}
}
