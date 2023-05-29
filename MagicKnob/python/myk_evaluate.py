import myk_data
import soundfile
import torch 
import numpy as np

def add_param(indat, param):

  newdat = np.empty([len(indat), 2])
  newdat[:,0] = np.squeeze(indat)
  newdat[:,1] = param
  return newdat


def run_file_through_model(model, infile, outfile, samplerate=44100):
    """
    read the sent file from disk, pass it through the model
    and back out to the other file 
    """
    param_list = [0, 0.5, 1]
    indata = myk_data.load_wav_file(infile, want_samplerate=samplerate)

    for p in param_list:
        
        indata_parametric = add_param(indata, p)
        outputs = model.forward(torch.tensor(indata_parametric, dtype=torch.float32))
        outputs = outputs.cpu().detach().numpy()
        outfile_temp = outfile[0: len(outfile)-4]
        outfile_def = outfile_temp+ "-" + f"{p}.wav"
        soundfile.write(outfile_def, outputs, samplerate)