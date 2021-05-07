using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AudioMixer
{
    class KeyValue<K,V>
    {
        private K key;
        private V value;
        private String text = "";
        public KeyValue()
        {
        }
        public KeyValue(K key, V value, String text)
        {
            this.key = key;
            this.value = value;
            this.text = text;
        }

        public KeyValue(K key, V value) :this(key, value, "")
        {
            this.text = key.ToString();
        }

        public K Key
        {
            get { return this.key; }
            set { this.key = value; }
        }
        public V Value
        {
            get { return this.value; }
            set { this.value= value; }
        }
        public override string ToString()
        {
            return this.text;
        }
    }
}
