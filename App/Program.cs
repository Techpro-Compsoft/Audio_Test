using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Avf;


namespace App
{
    class Program
    {
        static void Main(string[] args)
        {
            Dictionary<String, String> inputs = new Dictionary<String, String>();
            Dictionary<String, String> outputs = new Dictionary<String, String>();

            inputs.Add("INPUT", "D:\\WORKSPACE\\AMIX\\amix\\samples\\ABC\\123.mp3");

            String A2Z = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

            foreach(Char ch in A2Z)
            {
                inputs.Add(ch.ToString(), String.Format("D:\\WORKSPACE\\AMIX\\amix\\samples\\ABC\\{0}.mp3", ch.ToString()));
            }

            outputs.Add("OUT", "D:\\WORKSPACE\\AMIX\\amix\\samples\\output.mp3");

            List<String> sequence = new List <String>();

            String batch = A2Z  + "ABCDEF";
            for (int i = 0; i < 32; i++)
            {
                foreach(Char ch in batch) {
                    sequence.Add(ch.ToString());
                }
            }

            sequence = new List<String>{ "A", "B", "C", "D", "E", "F", "G", "H"};


            Mixer mixer = new Mixer(new Options(), inputs, outputs, sequence);
            mixer.Open();
            while (mixer.Next()) ;
            mixer.Close();

            Console.WriteLine("Okay!");
        }
    }
}
