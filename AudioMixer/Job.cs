using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Threading.Tasks;
using System.Drawing;
using Avf;


namespace AudioMixer
{
    class Job
    {
        int max_files_per_batch = 30;
        int max_inputs_per_batch = 1024;
        Options options = new Options();
        Dictionary<String, String> inputs = null;
        String output = null;
        String sequence = null;
        String directory = null;
        Log log = null;
        List<List<Mixer>> stages = new List<List<Mixer>>();
        double progress = 0;
        int stage = -1;
        int mixer = 0;
        double multiplier = 0;
        int total = 0;
        int offset = 0;
        public Job(Log log, Dictionary<String, String> inputs, String output, string sequence)
        {
            this.log = log;
            this.inputs = inputs;
            this.output = output;
            this.sequence = sequence;
        }
        private void DeleteFile(String path)
        {
            try
            {
                if (File.Exists(path))
                {
                    File.Delete(path);
                }
            }
            catch { }

        }
        public Options NewOptions()
        {
            Options options = this.options.Clone();
            options.Parameters.Metadata.Clear();
            return options;
        }
        public void LogDictionary(Log log, int tabs, Color color, Dictionary<String, String> map)
        {
            int align = 0;
            foreach (String key in map.Keys)
            {
                if (align < key.Length) align = key.Length;
            }
            foreach (KeyValuePair<String, String> kv in map)
            {
                log.WriteLine(tabs, color, "{0," + (-align) + "} : {1}", kv.Key, kv.Value);
            }
        }
        public void LogParameters(Log log, int tabs, Color color, Parameters parameters)
        {
            String duration = Util.TimeStamp(parameters.Duration);
            String mode = parameters.BitRateMode.ToString().ToLowerInvariant();
            String samplerate = Util.SampleRate(parameters.SampleRate);
            String bitrate = Util.BitRate(parameters.BitRate);
            String channels = Util.Channels(parameters.Channels);

            log.WriteLine(tabs, color, "Duration: {0}, channels: {1}, samplerate: {2}, bitrate: {3}", duration, channels, samplerate, bitrate);

            if (parameters.Metadata.Count > 0)
            {
                tabs++;
                log.WriteLine(tabs, color, "MetaData:");
                tabs++;
                LogDictionary(log, tabs, color, parameters.Metadata);
            }
        }
        public void LogJob(Log log, int tabs, Color color,  Mixer mixer)
        {
            foreach (String output in mixer.Outputs)
            {
                log.WriteLine(tabs, color, "Output: '{0}'", output);
            }
            tabs++;
            StringBuilder builder = new StringBuilder();

            foreach (String letter in mixer.Sequence)
            {
                builder.Append(letter);
            }
            List<String> lines = Util.FormatSequence(builder.ToString());

            if (lines.Count > 1 || lines[0].Length > 10)
            {
                log.WriteLine(tabs, color, "Sequence:");
                tabs++;

                foreach (String line in lines)
                {
                    log.WriteLine(tabs, color, line);
                }
            }
            else
            {
                log.WriteLine(tabs, color, "Sequence: '{0}'", lines[0]);
            }

        }
        public void LogSuccess(Log log, int tabs, Color color, String url)
        {
            Parameters parameters = Parameters.Read(url);
            if (parameters == null)
            {
                throw new Exception(String.Format("Unable to read valid data from {0}.", url));
            }
            log.WriteLine(tabs, color, "Success:",url);
            tabs++;
            log.WriteLine(tabs, color, "Output: '{0}'",url);
            tabs++;
            LogParameters(log, tabs, color, parameters);

        }
        public void LogBatchFirst(Log log, int tabs, Color color, int offset, int total, Mixer mixer)
        {
            log.WriteLine(tabs, color, "Batch #{0}, of {1} :", offset + 1, total);
            tabs++;
            foreach (String output in mixer.Outputs)
            {
                log.WriteLine(tabs, color, "Output: '{0}'", output);
            }
            tabs++;
            StringBuilder builder = new StringBuilder();

            foreach (String letter in mixer.Sequence)
            {
                builder.Append(letter);
            }
            List<String> lines = Util.FormatSequence(builder.ToString());

            if (lines.Count > 1 || lines[0].Length > 10)
            {
                log.WriteLine(tabs, color, "Sequence:");
                tabs++;

                foreach (String line in lines)
                {
                    log.WriteLine(tabs, color, line);
                }
            }
            else
            {
                log.WriteLine(tabs, color, "Sequence: '{0}'", lines[0]);
            }
        }
        public void LogBatchRest(Log log, int tabs, Color color, int offset, int total, Mixer mixer)
        {
            log.WriteLine(tabs, color, "Batch #{0}, of {1} :", offset + 1, total);
            tabs++;
            foreach (String output in mixer.Outputs)
            {
                log.WriteLine(tabs, color, "Output: '{0}'", output);
            }
            tabs++;
            String[] inputs = mixer.Inputs;
            for (int i = 0; i < inputs.Length; i++)
            {
                log.WriteLine(tabs, color, "Input #{0} : '{1}'", i + 1, inputs[i]);
            }
        }
        public void Open()
        {

            int tabs = 0;
            Color color = Magic.Cyan;
            log.Newline();
            List<String> lines = Util.FormatSequence(this.sequence);
            if (lines.Count == 1 && lines[0].Length <= 10)
            {
                log.WriteLine(tabs, color, "SEQUENCE: '{0}'", lines[0]);
            }
            else
            {
                log.WriteLine(tabs, color, "SEQUENCE:");
                tabs++;
                foreach (String line in lines)
                {
                    log.WriteLine(tabs, color, line);
                }
                tabs--;
            }
            color = Magic.Yellow;
            log.Newline();
            log.WriteLine(tabs, color, "LETTERS:");
             tabs++;
            for (Char ch = 'A'; ch <= 'Z'; ch++)
            {
                String label = ch.ToString();
                if (this.inputs.ContainsKey(label))
                {
                    String url = this.inputs[label];
                    Parameters parameters = Parameters.Read(url);
                    if (parameters == null)
                    {
                        throw new Exception(String.Format("Unable to read valid data from {0}.", url));
                    }
                    log.WriteLine(tabs, color, "[{0}] -> '{1}':", label, url);
                    LogParameters(log, tabs + 1, color, parameters);
                }
            }
            tabs--;
            if (this.sequence.Length <= max_inputs_per_batch && this.inputs.Count <= max_files_per_batch)
            {
                List<Mixer> stage = new List<Mixer>();
                Dictionary<String, String> outputs = new Dictionary<string, string>();
                outputs["Out"] = output;
                List<String> sequence = new List<String>();

                foreach (Char ch in this.sequence)
                {
                    sequence.Add(ch.ToString());
                }
                stage.Add(new Mixer(this.options.Clone(), this.inputs, outputs, sequence));
                this.stages.Add(stage);
            }
            else
            {
                while (true)
                {
                    this.directory = Path.Combine(Path.GetTempPath(), Guid.NewGuid().ToString());
                    if (!Directory.Exists(this.directory))
                    {
                        try
                        {
                            Directory.CreateDirectory(this.directory);
                            break;
                        }
                        catch { }
                    }
                }
                int n = 1;
                List<String> source = new List<String>();
                List<String> destination = new List<String>();
                {
                    List<Mixer> stage = new List<Mixer>();
                    Dictionary<String, String> inputs = new Dictionary<string, string>();
                    Dictionary<String, String> outputs = new Dictionary<string, string>();
                    List<String> sequence = new List<String>();

                    for (int i = 0; i < this.sequence.Length; i++)
                    {
                        String key = this.sequence[i].ToString();

                        if ((sequence.Count == max_inputs_per_batch) || ((inputs.Count == max_files_per_batch) && !inputs.ContainsKey(key)))
                        {
                            String output = Path.Combine(this.directory, String.Format("{0}.mp3", n++));
                            outputs["Out"] = output;
                            source.Add(output);
                            stage.Add(new Mixer(NewOptions(), inputs, outputs, sequence));

                            sequence = new List<String>();
                            inputs = new Dictionary<string, string>();
                            outputs = new Dictionary<string, string>();
                        }
                        sequence.Add(key);
                        inputs[key] = this.inputs[key];

                        if (i == this.sequence.Length - 1)
                        {
                            String output = Path.Combine(this.directory, String.Format("{0}.mp3", n++));
                            outputs["Out"] = output;
                            source.Add(output);
                            stage.Add(new Mixer(NewOptions(), inputs, outputs, sequence));
                        }
                    }
                    this.stages.Add(stage);
                }

                int limit = Math.Min(max_inputs_per_batch, max_files_per_batch);

                while (source.Count > 0)
                {
                    List<Mixer> stage = new List<Mixer>();
                    for (int i = 0; i < source.Count; i += limit)
                    {

                        int count = (source.Count - i) < limit ? (source.Count - i) : limit;

                        Dictionary<String, String> outputs = new Dictionary<string, string>();
                        Dictionary<String, String> inputs = new Dictionary<string, string>();
                        List<String> sequence = new List<String>();

                        for (int j = 0; j < count; j++)
                        {
                            String label = String.Format("In{0}", j);
                            inputs[label] = source[i + j];
                            sequence.Add(label);
                        }
                        if (source.Count > limit)
                        {
                            String output = Path.Combine(this.directory, String.Format("{0}.mp3", n++));
                            outputs["Out"] = output;
                            destination.Add(output);
                            stage.Add(new Mixer(NewOptions(), inputs, outputs, sequence));
                        }
                        else
                        {
                            outputs["Out"] = output;
                            stage.Add(new Mixer(this.options.Clone(), inputs, outputs, sequence));
                        }
                    }
                    this.stages.Add(stage);
                    source = destination;
                    destination = new List<String>();
                }
            }
            foreach (List<Mixer> stage in stages)
            {
                this.total += stage.Count;
            }

            this.multiplier = 1.0 / this.total;

            log.Newline();
            color = Magic.Green;
            log.WriteLine(tabs, color, "JOB:");
            if (this.total > 1)
            {
                tabs++;
                log.WriteLine(tabs, color, "> Spitting job into {0} batches", this.total);
                log.WriteLine(tabs, color, "> Using '{0}' as the temporary directory", this.directory);
                tabs--;
            }
            tabs++;
            log.WriteLine(tabs, color, "Start:");

        }
        public bool Next()
        {
            Color color = Magic.Green;
            int tabs = 1;

            bool result = false;
            if (this.stage < this.stages.Count)
            {
                result = true;

                bool first = (this.stage < 0);
                if (first)
                {
                    this.stage = this.mixer = 0;
                    this.stages[this.stage][this.mixer].Open();

                    if (this.total == 1)
                    {
                        LogJob(this.log, tabs + 1, color, this.stages[this.stage][this.mixer]);
                    }
                    else
                    {
                        LogBatchFirst(this.log, tabs + 1, color, this.offset, this.total, this.stages[this.stage][this.mixer]);
                    }
                }
                if (!this.stages[this.stage][this.mixer].Next())
                {
                    this.stages[this.stage][this.mixer].Close();
                    this.progress += this.multiplier;
                    if (this.stage > 0 )
                    {
                        foreach (String input in this.stages[this.stage][this.mixer].Inputs)
                        {
                            DeleteFile(input);
                        }
                    }
                    if (mixer < this.stages[this.stage].Count - 1)
                    {
                        this.mixer++;
                    }
                    else
                    {
                        this.stage++;
                        this.mixer = 0;
                    }
                    this.offset++;
                    if (stage < this.stages.Count)
                    {
                        this.stages[this.stage][this.mixer].Open();

                        if (this.stage == 0)
                        {
                            LogBatchFirst(this.log, tabs + 1, color, this.offset, this.total, this.stages[this.stage][this.mixer]);
                        }
                        else if (this.stage < this.stages.Count)
                        {
                            LogBatchRest(this.log, tabs + 1, color, this.offset, this.total, this.stages[this.stage][this.mixer]);
                        }
                    }
                    else
                    {
                        result = false;
                        LogSuccess(this.log, tabs, color, this.output);
                    }
                }
            }
            return result;
        }
        public void Close()
        {
            foreach (List<Mixer> stage in this.stages)
            {
                foreach (Mixer mixer in stage)
                {
                    mixer.Close();
                }
            }
            if (this.directory != null)
            {
                try
                {
                    if (Directory.Exists(this.directory))
                    {
                        Directory.Delete(this.directory, true);
                    }
                }
                catch { }
            }
        }
        public Log Log
        {
            get
            {
                return log;
            }
        }
        public int MaxFilesPerBatch
        {
            get
            {
                return max_files_per_batch;
            }
            set
            {
                max_files_per_batch = value;
            }
        }
        public int MaxInputsPerBatch
        {
            get
            {
                return max_inputs_per_batch;
            }
            set
            {
                max_inputs_per_batch = value;
            }
        }
        public Options Options
        {
            get
            {
                return options;
            }
        }
        public double Progress
        {
            get
            {
                if (stage >= 0 && stage < this.stages.Count && mixer >= 0 && mixer < this.stages[stage].Count)
                {
                    return this.progress + this.stages[stage][mixer].Progress * this.multiplier;
                }
                else
                {
                    return 0;
                }
            }
        }
    }
}
