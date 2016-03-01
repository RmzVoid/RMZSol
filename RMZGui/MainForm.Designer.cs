namespace RMZGui
{
	partial class MainForm
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.btnRead = new System.Windows.Forms.Button();
			this.btnWrite = new System.Windows.Forms.Button();
			this.lbFromClient = new System.Windows.Forms.ListBox();
			this.bgwRead = new System.ComponentModel.BackgroundWorker();
			this.SuspendLayout();
			// 
			// btnRead
			// 
			this.btnRead.Enabled = false;
			this.btnRead.Location = new System.Drawing.Point(13, 27);
			this.btnRead.Name = "btnRead";
			this.btnRead.Size = new System.Drawing.Size(75, 23);
			this.btnRead.TabIndex = 1;
			this.btnRead.Text = "Read";
			this.btnRead.UseVisualStyleBackColor = true;
			this.btnRead.Click += new System.EventHandler(this.btnRead_Click);
			// 
			// btnWrite
			// 
			this.btnWrite.Location = new System.Drawing.Point(94, 27);
			this.btnWrite.Name = "btnWrite";
			this.btnWrite.Size = new System.Drawing.Size(75, 23);
			this.btnWrite.TabIndex = 2;
			this.btnWrite.Text = "Write";
			this.btnWrite.UseVisualStyleBackColor = true;
			this.btnWrite.Click += new System.EventHandler(this.btnWrite_Click);
			// 
			// lbFromClient
			// 
			this.lbFromClient.FormattingEnabled = true;
			this.lbFromClient.Location = new System.Drawing.Point(13, 57);
			this.lbFromClient.Name = "lbFromClient";
			this.lbFromClient.Size = new System.Drawing.Size(316, 368);
			this.lbFromClient.TabIndex = 3;
			// 
			// MainForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(656, 437);
			this.Controls.Add(this.lbFromClient);
			this.Controls.Add(this.btnWrite);
			this.Controls.Add(this.btnRead);
			this.Name = "MainForm";
			this.Text = "RmzGui";
			this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Button btnRead;
		private System.Windows.Forms.Button btnWrite;
		private System.Windows.Forms.ListBox lbFromClient;
		private System.ComponentModel.BackgroundWorker bgwRead;
	}
}

