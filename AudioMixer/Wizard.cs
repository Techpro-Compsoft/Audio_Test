using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.IO;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Avf;

namespace AudioMixer
{
    public partial class Wizard : Form
    {
        BindingList<KeyValue<String, String>> files = new BindingList<KeyValue<String, String>>();
        BindingList<KeyValue<String, String>> tags = new BindingList<KeyValue<String, String>>();
        String location = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "AudioMixerSettings.xml");
        List<String> undos = new List<string>();
        DateTime start;
        bool closing = false;
        public Wizard()
        {
            InitializeComponent();
        }
        private void Wizard_Load(object sender, EventArgs e)
        {
            Reset();
            ReadSettings();

            //scInput.Panel2Collapsed = true;
            // scOutput.Panel2Collapsed = true;
            undos.Add("");
        }
        private void Reset()
        {
            this.cbInput.Text = "";
            this.cbInput.Items.Clear();
            this.gdFiles.DataSource = (object)this.files;
            for (char ch = 'A'; ch <= 'Z'; ++ch)
                this.files.Add(new KeyValue<string, string>(ch.ToString(), "", ch.ToString()));
            this.lbSequenceCharacters.Text = "0";
            this.tbSequence.Clear();
            this.cbOutput.Text = "";
            this.cbOutput.Items.Clear();
            this.cbBitRateMode.Items.Clear();
            foreach (int bitRateMode in Codec.BitRateModes)
                this.cbBitRateMode.Items.Add((object)(BitRateMode)bitRateMode);
            this.cbBitRateMode.SelectedIndex = 0;
            this.cbBitRate.Text = "";
            this.cbBitRate.Items.Clear();
            foreach (int bitRate in Codec.BitRates)
            {
                string text = Util.BitRate((long)bitRate);
                this.cbBitRate.Items.Add((object)new KeyValue<int, int>(bitRate, bitRate, text));
            }
            this.cbBitRate.SelectedIndex = 0;
            this.cbSampleRate.Text = "";
            this.cbSampleRate.Items.Clear();
            foreach (int sampleRate in Codec.SampleRates)
            {
                string text = Util.SampleRate(sampleRate);
                this.cbSampleRate.Items.Add((object)new KeyValue<int, int>(sampleRate, sampleRate, text));
            }
            this.cbSampleRate.SelectedIndex = 0;
            this.cbChannelLayout.Text = "";
            this.cbChannelLayout.Items.Clear();
            foreach (long channelLayout in Codec.ChannelLayouts)
                this.cbChannelLayout.Items.Add((object)(ChannelLayout)channelLayout);
            this.cbChannelLayout.SelectedIndex = 0;
            this.cxAbortOnError.Checked = false;
            this.cxNormalizeVolume.Checked = true;
            this.nuMaxInputs.Value = 1024M;
            this.nuMaxFiles.Value = 30M;
            this.tags.Clear();
            foreach (string tag in Codec.Tags)
                this.tags.Add(new KeyValue<string, string>(this.TagToName(tag), ""));
            this.gdTags.DataSource = (object)this.tags;
            this.rtLog.Clear();
            this.pgWork.Value = 0;
            this.lbElapsed.Text = "";
            this.lbRemaining.Text = "";
            this.lbPercentage.Text = "";
        }
        void ReadSettings()
        {
            Settings settings = Settings.Read(location);

            if (settings != null)
            {
                cbInput.Items.Clear();
                if (settings.Input != null)
                {
                    foreach (String path in settings.Input)
                    {
                        cbInput.Items.Add(path);
                    }
                    if (cbInput.Items.Count > 0)
                    {
                        cbInput.SelectedIndex = 0;
                    }
                }
                cbOutput.Items.Clear();

                if (settings.Output != null)
                {
                    foreach (String path in settings.Output)
                    {
                        cbOutput.Items.Add(path);
                    }
                    if (cbOutput.Items.Count > 0)
                    {
                        cbOutput.SelectedIndex = 0;
                    }
                }
                if (settings.BitRateMode >= 0 && settings.BitRateMode < cbBitRateMode.Items.Count)
                {
                    cbBitRateMode.SelectedIndex = settings.BitRateMode;
                }
                if (settings.BitRate >= 0 && settings.BitRate < cbBitRate.Items.Count)
                {
                    cbBitRate.SelectedIndex = settings.BitRate;
                }
                if (settings.SampleRate >= 0 && settings.SampleRate < cbSampleRate.Items.Count)
                {
                    cbSampleRate.SelectedIndex = settings.SampleRate;
                }
                if (settings.ChannelLayout >= 0 && settings.ChannelLayout < cbChannelLayout.Items.Count)
                {
                    cbChannelLayout.SelectedIndex = settings.ChannelLayout;
                }
                cxAbortOnError.Checked = settings.AbortOnError;
                cxNormalizeVolume.Checked = settings.NormalizeVolume;

                if (settings.MaxInputs > 1)
                {
                    nuMaxInputs.Value = settings.MaxInputs;
                }
                if (settings.MaxFiles > 1)
                {
                    nuMaxFiles.Value = settings.MaxFiles;
                }
            }
        }
        void WriteSettings()
        {
            Settings settings = new Settings();

            List<String> input = new List<string>();
            foreach (Object item in cbInput.Items)
            {
                input.Add(item.ToString());
            }
            settings.Input = input.ToArray<String>();

            List<String> output = new List<string>();
            foreach (Object item in cbOutput.Items)
            {
                output.Add(item.ToString());
            }
            settings.Output = output.ToArray<String>();

            settings.BitRateMode = cbBitRateMode.SelectedIndex;
            settings.BitRate = cbBitRate.SelectedIndex;
            settings.SampleRate = cbSampleRate.SelectedIndex;
            settings.ChannelLayout = cbChannelLayout.SelectedIndex;

            settings.MaxInputs = Decimal.ToInt32(nuMaxInputs.Value);
            settings.MaxFiles = Decimal.ToInt32(nuMaxFiles.Value);

            Settings.Write(location, settings);
        }
        private void fileTable_SelectionChanged(object sender, EventArgs e)
        {
            gdFiles.ClearSelection();
        }

        private void tagTable_SelectionChanged(object sender, EventArgs e)
        {
            gdTags.ClearSelection();
        }
        private String TagToName(String value)
        {
            String[] parts = value.Split('_');
            StringBuilder builder = new StringBuilder();
            if (value != null && value.Length > 0)
            {
                value = value.Replace('_', ' ');
                value = value[0].ToString().ToUpperInvariant() + value.Substring(1);
            }
            foreach (String part in parts)
            {
                String word = part[0].ToString().ToUpperInvariant();
                if (part.Length > 1) word += part.Substring(1);
                if (builder.Length > 0)
                {
                    builder.Append(" ");
                }
                builder.Append(word);
            }
            return builder.ToString();
        }
        private void Wizard_FormClosed(object sender, FormClosedEventArgs e)
        {
            WriteSettings();
        }
        private void AddPath(ComboBox combo, String path)
        {
            int history = 10;
            if (combo.Items.Contains(path))
            {
                combo.Items.Remove(path);
            }
            combo.Items.Insert(0, path);

            while (combo.Items.Count > history)
            {
                combo.Items.RemoveAt(history);
            }
        }
        private void btnOutputBrowse_Click(object sender, EventArgs e)
        {
            if (DialogResult.OK == sfOutputSave.ShowDialog())
            {
                String path = sfOutputSave.FileName;
                if (!path.ToLowerInvariant().EndsWith(".mp3"))
                {
                    path += ".mp3";
                }
                AddPath(cbOutput, path);
                cbOutput.SelectedIndex = 0;
            }
        }
        private bool LoadDirectory(String path)
        {
            bool result = true;
            Dictionary<String, KeyValue<String, String>> map = new Dictionary<String, KeyValue<String, String>>();
            foreach (KeyValue<String, String> file in files)
            {
                file.Value = "";
                map[file.Key] = file;
            }
            try
            {
                String[] dir = Directory.GetFiles(path);
                foreach (String file in dir)
                {
                    String name = Path.GetFileName(file);
                    if (name.ToLowerInvariant().EndsWith(".mp3"))
                    {
                        String key = name[0].ToString();
                        if (name[0] >= 'A' && name[0] <= 'Z')
                        {
                            int index = name[0] - 'A';

                            if (files[index].Value == "")
                            {
                                files[index].Value = file;
                            }
                        }
                    }
                }
            }
            catch { }
            gdFiles.Update();
            gdFiles.Refresh();
            return result; ;
        }
        private void cbInput_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
            {
                String path = cbInput.Text;
                if (path.Length > 0)
                {
                    AddPath(cbInput, path);
                    cbInput.SelectedIndex = 0;
                }
            }
        }
        private void cbOutput_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
            {
                String path = cbOutput.Text;
                if (path.Length > 0)
                {
                    AddPath(cbOutput, path);
                    cbOutput.SelectedIndex = 0;
                }
            }

        }
        private void btnInputBrowse_Click(object sender, EventArgs e)
        {
            if (DialogResult.OK == fbInputBrowse.ShowDialog())
            {
                String path = fbInputBrowse.SelectedPath;
                AddPath(cbInput, path);
                cbInput.SelectedIndex = 0;
            }
        }
        private void cbInput_SelectedIndexChanged(object sender, EventArgs e)
        {
            String path = cbInput.SelectedItem.ToString();
            if (cbInput.SelectedIndex == 0)
            {
                LoadDirectory(path);
            }
            else if (cbInput.SelectedIndex > 0)
            {
                AddPath(cbInput, path);
                cbInput.SelectedIndex = 0;
            }
        }
        private void btnSequenceClear_Click(object sender, EventArgs e)
        {
            tbSequence.Clear();
        }
        private void btnSequenceArrange_Click(object sender, EventArgs e)
        {
            tbSequence.Lines = Util.FormatSequence(tbSequence.Text).ToArray();
        }
        private void btnSequenceCaptialize_Click(object sender, EventArgs e)
        {
            tbSequence.Text = tbSequence.Text.ToUpperInvariant();
        }
        private void btnSequenceUndo_Click(object sender, EventArgs e)
        {
            if (undos.Count > 1)
            {
                String undo = undos[1];
                undos.RemoveAt(0);
                undos.RemoveAt(0);
                tbSequence.Text = undo;
            }
        }
        private void tbSequence_TextChanged(object sender, EventArgs e)
        {
            int count = 0;
            foreach (Char ch in tbSequence.Text)
            {
                if (ch >= 'A' && ch <= 'Z')
                {
                    count++;
                }
            }
            lbSequenceCharacters.Text = count.ToString();
            undos.Insert(0, tbSequence.Text);
            while (undos.Count > 10)
            {
                undos.RemoveAt(undos.Count - 1);
            }
        }
        private void btnTagsClear_Click(object sender, EventArgs e)
        {
            foreach (KeyValue<String, String> keyvalue in tags)
            {
                keyvalue.Value = "";
            }
            gdTags.Update();
            gdTags.Refresh();
        }
        private void ErrorMessage(String message)
        {
            MessageBox.Show(this, message, "Error!", MessageBoxButtons.OK, MessageBoxIcon.Stop);
        }
        private void btnNext_Click(object sender, EventArgs e)
        {
            if (cbInput.Text.Length == 0)
            {
                String message = "Please select a location to read the mp3 from.";
                ErrorMessage(message);
                return;
            }
            StringBuilder builder = new StringBuilder();
            foreach (Char ch in tbSequence.Text)
            {
                if (ch >= 'A' && ch <= 'Z')
                {
                    builder.Append(ch);
                    int index = ch - 'A';
                    if (files[index].Value.Length == 0)
                    {
                        String message = String.Format("The letter ({0}) in the letter sequence is not mapped to a mp3 file.", ch);
                        ErrorMessage(message);
                        return;
                    }
                }
                else if (Char.IsWhiteSpace(ch))
                {
                }
                else
                {
                    String message = "The letter sequence contains characters that are not capital letters.";
                    ErrorMessage(message);
                    return;
                }
            }
            if (builder.Length == 0)
            {
                String message = "Please enter a letter sequence to mix.";
                ErrorMessage(message);
                return;
            }
            scOutput.Panel2Collapsed = true;
            scInput.Panel1Collapsed = true;
        }
        private void btnBack_Click(object sender, EventArgs e)
        {
            scInput.Panel2Collapsed = true;
            scOutput.Panel2Collapsed = true;
        }

        private void cbInput_TextUpdate(object sender, EventArgs e)
        {
            LoadDirectory(cbInput.Text);
        }
        private void btnStart_Click(object sender, EventArgs e)
        {
            if (cbOutput.Text.Length == 0)
            {
                String message = "Please select a location to write the resulting mp3 to.";
                ErrorMessage(message);
                return;
            }
            else if (!cbOutput.Text.ToLowerInvariant().EndsWith(".mp3"))
            {
                String message = "The filename of the output must end with '.mp3'.";
                ErrorMessage(message);
                return;
            }
            Dictionary<String, String> inputs = new Dictionary<string, string>();
            String output = cbOutput.Text;
            StringBuilder sequence = new StringBuilder();
            foreach (Char ch in tbSequence.Text)
            {
                if (ch >= 'A' && ch <= 'Z')
                {
                    sequence.Append(ch);
                    int index = ch - 'A';
                    inputs[ch.ToString()] = files[index].Value;
                }
            }

            Job job = new Job(new Log(bgWorker), inputs, output, sequence.ToString());

            job.MaxFilesPerBatch = Decimal.ToInt32(nuMaxFiles.Value);
            job.MaxInputsPerBatch = Decimal.ToInt32(nuMaxInputs.Value);

            job.Options.Ignore = !cxAbortOnError.Checked;
            job.Options.Normalize = cxNormalizeVolume.Checked;

            job.Options.Parameters.BitRate = ((KeyValue<int, int>)cbBitRate.SelectedItem).Value;
            job.Options.Parameters.SampleRate = ((KeyValue<int, int>)cbSampleRate.SelectedItem).Value;
            job.Options.Parameters.BitRateMode = ((BitRateMode)cbBitRateMode.SelectedItem);
            job.Options.Parameters.ChannelLayout = (long)((ChannelLayout)cbChannelLayout.SelectedItem);

            String[] tags = Codec.Tags;
            if (tags.Length == this.tags.Count)
            {
                for (int i = 0; i < this.tags.Count; i++)
                {
                    if (this.tags[i].Value.Length > 0)
                    {
                        job.Options.Parameters.Metadata.Add(tags[i], this.tags[i].Value);
                    }
                }
            }
            if (!bgWorker.IsBusy)
            {
                pgWork.Value = 0;
                lbElapsed.Text = "";
                lbPercentage.Text = "";
                lbRemaining.Text = "";
                gbProgress.Visible = true;
                rtLog.Clear();
                btnCancel.Text = "Cancel";
                start = DateTime.Now;
                bgWorker.RunWorkerAsync(job);

            }
            this.scOutput.Panel1Collapsed = true;
            this.scInput.Panel1Collapsed = true;
        }
        private void gdTags_CurrentCellChanged(object sender, EventArgs e)
        {
            if (gdTags.CurrentCell != null)
            {
                if (gdTags.CurrentCell.RowIndex >= 0 && gdTags.CurrentCell.ColumnIndex == 1)
                {
                    gdTags.BeginEdit(true);
                }
            }
        }
        private void bgWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (e.Cancelled || e.Error != null)
            {
                pgWork.Value = 0;
                lbPercentage.Text = "";
                lbElapsed.Text = "";
                lbRemaining.Text = "";
                gbProgress.Visible = false;

                int tabs = 1;
                Color color = Magic.Red;
                WriteLine(tabs, color, "Error:");
                tabs++;

                if (e.Cancelled)
                {
                    WriteLine(tabs, color, "Reason: The job was cancelled by the user.");
                }
                else 
                {
                    WriteLine(tabs, color, "Reason: {0}", e.Error.Message);
                    ErrorMessage(e.Error.Message);
                }
            }
            else
            {
                pgWork.Value = pgWork.Maximum;
                lbPercentage.Text = "100.0%";
                lbElapsed.Text = "";
                lbRemaining.Text = "";
            }

            btnCancel.Text = "Okay";
            btnCancel.Enabled = true;

            if (this.closing)
            {
                this.Close();
            }


        }
        private void Write(Color color, String message)
        {
            rtLog.SelectionStart = rtLog.TextLength;
            rtLog.SelectionLength = 0;
            rtLog.SelectionColor = color;
            rtLog.AppendText(message);
            rtLog.SelectionStart = rtLog.TextLength;
            rtLog.SelectionLength = 0;
            rtLog.ScrollToCaret();

        }

        public void WriteLine(int tabs, Color color, String format, params Object[] args)
        {
            String message = String.Format("{0}{1}\n", Util.Tabs(tabs), String.Format(format, args));
            Write(color, message);
        }
        public void Newline()
        {
            Write(Color.Transparent, "\n");
        }


        private void bgWorker_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            if (e.UserState != null)
            {
                if (e.UserState is Message)
                {
                    Message message = (Message)e.UserState;
                    Write( message.Color,message.Text);
                }
                else if(e.UserState is String)
                {
                    String message = (String)e.UserState;
                    Write(Color.Transparent, message);
                }
                else if (e.UserState is Double)
                {
                    double progress = (Double)e.UserState;
                    int bar = Convert.ToInt32(progress * pgWork.Maximum);
                    if (bar > pgWork.Maximum) bar = pgWork.Maximum;
                    pgWork.Value = bar;
                    int seconds = Convert.ToInt32((DateTime.Now - start).TotalSeconds);
                    if (progress > 0.0001 && seconds >= 1)
                    {
                        lbElapsed.Text = String.Format("Elapsed: {0}", Util.TimeStamp(seconds));
                        double percentage = progress * 100;
                        if (percentage < 0.1) percentage = 1;
                        if (percentage > 99.9) percentage = 99;
                        lbPercentage.Text = String.Format("{0:F1}%", percentage);

                        int remaining = Convert.ToInt32(seconds / progress - seconds);
                        int value = remaining / 60 + 1;

                        int hours = value / 60;
                        int minutes = value % 60;

                        StringBuilder builder = new StringBuilder();

                        builder.Append("Less than ");

                        if(hours > 0)
                        {
                            builder.AppendFormat("{0} hour{1} and ", hours, Util.Plural(hours));
                        }
                        builder.AppendFormat("{0} minute{1} remaining", minutes, Util.Plural(minutes));

                        lbRemaining.Text = builder.ToString();
                    }
                    else
                    {
                        lbElapsed.Text = "";
                        lbPercentage.Text = "";
                        lbRemaining.Text = "";
                    }
                }
            }
        }
        private void bgWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            DateTime last = DateTime.Now;
            Job job = (Job)e.Argument;
            try
            {
                job.Open();
                while (true)
                {
                    if (bgWorker.CancellationPending)
                    {
                        e.Cancel = true;
                        break;
                    }
                    else
                    {
                        if (job.Next())
                        {
                            DateTime current = DateTime.Now;
                            if ((current - last).TotalMilliseconds > 400)
                            {
                                last = current;
                                bgWorker.ReportProgress(0, job.Progress);
                            }
                        }
                        else
                        {
                            e.Cancel = false;
                            break;
                        }
                    }
                }
            }
            catch (SourceException ex)
            {
                throw new Exception(String.Format("Unable to read valid data from {0}.", ex.Url));
            }
            catch (SinkException ex)
            {
                throw new Exception(String.Format("Unable to write to {0}.", ex.Url));
            }
            catch (FilterGraphException)
            {
                throw new Exception(String.Format("Encoding error."));
            }
            catch (MixerException)
            {
                throw new Exception(String.Format("Mixing error."));
            }
            catch 
            {
            }
            finally
            {
                job.Close();
            }
        }
        private void Wizard_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (bgWorker.IsBusy)
            {
                e.Cancel = true;
                bgWorker.CancelAsync();
                this.Visible = false;
            }
        }
        private void btnCancel_Click(object sender, EventArgs e)
        {
            if (bgWorker.IsBusy)
            {
                bgWorker.CancelAsync();
                btnCancel.Enabled = false;
            }
            else
            {
                this.scInput.Panel2Collapsed = true;
                this.scOutput.Panel2Collapsed = true;
            }
        }

        private void cbBitRateMode_SelectedIndexChanged(object sender, EventArgs e)
        {

        }
    }
}
