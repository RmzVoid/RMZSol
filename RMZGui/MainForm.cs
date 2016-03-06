using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
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
	public partial class MainForm : Form
	{
		private DeviceIO device;
		private CancellationTokenSource cancel = new CancellationTokenSource();
		private PacketDispatcher dispatcher;

		public MainForm()
		{
			InitializeComponent();

			try
			{
				dispatcher = new PacketDispatcher();
				device = new DeviceIO(@"\\.\rmzdrv", cancel.Token, dispatcher);
			}
			catch (Exception exception)
			{
				MessageBox.Show(exception.Message, exception.Source, MessageBoxButtons.OK, MessageBoxIcon.Error);
			}
		}

		private void btnRead_Click(object sender, EventArgs e)
		{
		}

		private void btnWrite_Click(object sender, EventArgs e)
		{
		}

		private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
		{
			cancel.Cancel();
		}
	}
}
