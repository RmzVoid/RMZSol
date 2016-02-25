using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace RMZGui
{
	public partial class MainForm : Form
	{
		private FileStream readStream;

		public MainForm()
		{
			InitializeComponent();

			try
			{
				IntPtr hDevice = Win32.CreateFile(@"\\.\rmzdrv",
					Win32.FileAccess.GenericRead,
					Win32.FileShare.Read,
					IntPtr.Zero,
					Win32.CreationDisposition.OpenExisting,
					Win32.FileAttributes.Normal,
					IntPtr.Zero);

				Marshal.ThrowExceptionForHR(Marshal.GetHRForLastWin32Error());

				//readStream = new FileStream(@"\\.\device\rmzdrv", FileMode.Open, FileAccess.Read, FileShare.Read, 1024, true);
			}
			catch (Exception e)
			{
				MessageBox.Show(e.Message, e.Source, MessageBoxButtons.OK, MessageBoxIcon.Error);
			}
		}

		private void btnRead_Click(object sender, EventArgs e)
		{

		}
	}
}
