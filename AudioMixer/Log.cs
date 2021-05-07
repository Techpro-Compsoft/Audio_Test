using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.ComponentModel;
using System.Drawing;
using Avf;

namespace AudioMixer
{
    class Log 
    {
        BackgroundWorker worker = null;
        public Log(BackgroundWorker worker)
        {
            this.worker = worker;
        }
        private void Write(Color color, String message)
        {
            worker.ReportProgress(0, new Message(message , color));
        }
        public void WriteLine(int tabs, Color color,String format, params Object[] args)
        {
            String message = String.Format("{0}{1}\n", Util.Tabs(tabs), String.Format(format, args));
            worker.ReportProgress(0, new Message(message, color));
        }
        public void Newline()
        {
            worker.ReportProgress(0, "\n");
        }
    }
}
