import myk_data
import soundfile
import torch 
import numpy as np


def run_file_through_model(model, infile, outfile, samplerate=44100):
    """
    read the sent file from disk, pass it through the model
    and back out to the other file 
    """

    indata = myk_data.load_wav_file(infile, want_samplerate=samplerate)

    outputs = model.forward(torch.tensor(indata, dtype=torch.float32))
    outputs = outputs.cpu().detach().numpy()
    outfile_temp = outfile[0: len(outfile)-4]
    outfile_def = f"{outfile_temp}.wav"
    soundfile.write(outfile_def, outputs, samplerate)