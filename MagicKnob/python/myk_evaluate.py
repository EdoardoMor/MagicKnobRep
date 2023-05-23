import myk_data
import soundfile
import torch 

def run_file_through_model(model, infile, outfile, samplerate=44100):
    """
    read the sent file from disk, pass it through the model
    and back out to the other file 
    """
    indata = myk_data.load_wav_file(infile, want_samplerate=samplerate)
    outputs = model.forward(torch.tensor(indata))
    outputs = outputs.cpu().detach().numpy()
    soundfile.write(outfile, outputs, samplerate)
    
