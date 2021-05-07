using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Drawing;
using Avf;

namespace AudioMixer
{
    class Util
    {
        public static String Plural(int n)
        {
             return  n > 1 ? "s" : "";
        }
        public static String BitRate(long bitrate)
        {
            return String.Format("{0} {1}", ((double)bitrate) / 1000, "Kbps");
        }
        public static String SampleRate(int samplerate)
        {
            return  String.Format("{0} {1}", ((double)samplerate) / 1000, "Khz");
        }
        public static String TimeStamp(double seconds) 
        {
            int sec = Convert.ToInt32(seconds);
            return String.Format(" {0:D2}:{1:D2}:{2:D2}", sec/ (60 * 60), (sec % (60 * 60)) / 60, sec % 60);
        }
        public static String Channels(int channels)
        {
           return (channels == 1) ? "mono" : ((channels ==2) ? "stereo" : channels.ToString());
        }
        public static String Tabs(int n)
        {
            String tab = "  ";
            StringBuilder builder = new StringBuilder();
            for(int i = 0; i < n; i++)
            {
                builder.Append(tab);
            }
            return builder.ToString();
        }
        public static List<String> FormatSequence(String sequence)
        {
            List<String> lines = new List<string>();
            StringBuilder line = new StringBuilder();
            StringBuilder builder = new StringBuilder();

            foreach (Char ch in sequence)
            {
                if (ch >= 'A' && ch <= 'Z')
                {
                    builder.Append(ch);
                    if (builder.Length == 10)
                    {
                        if (line.Length > 0)
                        {
                            line.Append(" ");
                        }
                        line.Append(builder.ToString());
                        builder.Clear();

                        if (line.Length + 1 == 55)
                        {
                            if (lines.Count > 0 && (lines.Count + 1) % 11 == 0)
                            {
                                lines.Add("");
                            }
                            lines.Add(line.ToString());
                            line.Clear();
                        }
                    }
                }
            }
            if (builder.Length > 0)
            {
                if (line.Length > 0)
                {
                    line.Append(" ");
                }
                line.Append(builder.ToString());
                builder.Clear();
            }

            if (line.Length > 0)
            {
                if (lines.Count > 0 && (lines.Count + 1) % 11 == 0)
                {
                    lines.Add("");
                }
                lines.Add(line.ToString());
                line.Clear();
            }
            return lines;
        }
    }
}
