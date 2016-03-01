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
		// size of buffer for read and write
		private const int bufferSize = 16384;

		// stream for data exchange with device
		private FileStream deviceStream = null;

		// token for cancelling
		private CancellationToken cancelToken;

		Thread readingThread = new Thread(DoRead);

		public DeviceIO(string devicePath, CancellationToken cancel)
		{
			//
			// Open device
			//SafeFileHandle deviceHandle = Win32.CreateFile(devicePath,
			//	Win32.FileAccess.GenericRead | Win32.FileAccess.GenericWrite,
			//	Win32.FileShare.None,
			//	IntPtr.Zero,
			//	Win32.CreationDisposition.OpenExisting,
			//	Win32.FileAttributes.Normal | Win32.FileAttributes.Overlapped,
			//	IntPtr.Zero);

			//if (deviceHandle.IsInvalid)
			//	Marshal.ThrowExceptionForHR(Marshal.GetHRForLastWin32Error());

			////
			//// Open stream for data flow
			//deviceStream = new FileStream(deviceHandle, FileAccess.ReadWrite, bufferSize, true);

			cancelToken = cancel;

			readingThread.Start(this);
		}

		private static void DoRead(object obj)
		{
			DeviceIO device = obj as DeviceIO;
			device.cancelToken.WaitHandle.WaitOne();
			MessageBox.Show("1");
		}
	}
}
