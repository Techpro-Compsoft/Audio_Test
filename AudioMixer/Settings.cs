using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.IO;
using System.Xml.Serialization;

namespace AudioMixer
{
    public class Settings
    {
        public String[] Input = new String[0];
        public String[] Output = new String[0];
        public int BitRateMode = 0;
        public int BitRate = 0;
        public int SampleRate = 0;
        public int ChannelLayout = 0;
        public bool AbortOnError = false;
        public bool NormalizeVolume = true;
        public int MaxInputs = 1024;
        public int MaxFiles = 1024;

        static public  bool Write(String url, Settings settings)
        {
            bool result = true;
            try
            {
                using (FileStream fs =  new FileStream(url, FileMode.Create))
                {
                    using (StreamWriter writer = new StreamWriter(fs, Encoding.UTF8))
                    {
                        XmlSerializer serializer = new XmlSerializer(typeof(Settings));
                        serializer.Serialize(writer, settings);
                        fs.Flush();
                    }
                }
            }
            catch 
            {
                result = false;
            }
            return result;
        }
        static public Settings Read(String url)
        {
            Settings settings = null;
            try
            {
                using (FileStream fs = File.OpenRead(url))
                {
                    using (StreamReader reader = new StreamReader(fs, Encoding.UTF8))
                    {
                        XmlSerializer serializer = new XmlSerializer(typeof(Settings));
                        settings = (Settings)serializer.Deserialize(reader);
                    }
                }
            }
            catch { }
            return settings; ;
        }
    }
}
