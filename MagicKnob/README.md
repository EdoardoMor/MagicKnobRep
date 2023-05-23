# Training an LSTM with torch, exporting and loading into rtneural for inference

```
# setup venv with torch, numpy,soundfile
python3 -m venv /put/venv/here
source /put/venv/here/bin/activate
# or on windows:
py -m venv /put/venv/here
/put/venv/here/Scripts/activate 
# install the bits
pip install torch numpy soundfile

# run the script to train a network
python 1_train_lstm.py
```

Edit the config on the script:
```
run_name="ht1"
audio_folder = "../data/audio_ht1"
batch_size = 50
```

It uses quite a few assertions to check things, read the messages carefully it it crashes.

The script automatically saves out examples of a test sound passed through the network as it runs

# Load the model into C++


