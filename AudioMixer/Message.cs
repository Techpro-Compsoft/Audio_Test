using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Drawing;

namespace AudioMixer
{
    class Message
    {
        string text;
        Color color = Color.White;
       public Message(String text, Color color)
        {
            this.text = text;
            this.color = color;
        }
        public String Text
        {
            get
            {
                return this.text;
            }
        }
        public Color Color
        {
            get
            {
                return this.color;
            }
        }
    }
}
