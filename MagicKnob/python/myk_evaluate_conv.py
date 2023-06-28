import myk_data_conv
import soundfile
import torch 
import numpy as np

def run_file_through_model(model, infile, outfile, samplerate=44100):
    """
    read the sent file from disk, pass it through the model
    and back out to the other file 
    """
    indata = myk_data_conv.load_wav_file(infile, want_samplerate=samplerate)
    outputs = model.forward(torch.tensor(indata))
    outputs = torch.squeeze(outputs)
    outputs = outputs.cpu().detach().numpy()
    # out_size = outputs.shape
    #print("OUTPUTS: " + str(out_size))
    soundfile.write(outfile, outputs, samplerate)