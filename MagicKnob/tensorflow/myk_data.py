import os 
#from scipy.io import wavfile
import numpy as np
import soundfile as sf
# import torch
# from torch.utils.data import TensorDataset
# from torch.utils.data import random_split
import tensorflow as tf
from sklearn.model_selection import train_test_split


def get_files_in_folder(folder, extension='.wav'):
    """
    returns a list of files in the sent folder with the sent extension
    """
    file_list = []
    for file in os.listdir(folder):
        if file.endswith(extension):
            file_list.append(os.path.join(folder, file))
    return file_list

def load_wav_file(filename, want_samplerate):
    """
    Load a WAV file using the soundfile module, resample to 44100 Hz, and return the first channel.
    """
    # Load the WAV file
    data, samplerate = sf.read(filename, dtype='float32')

    # Resample to 44100 Hz
    if samplerate != want_samplerate:
        print("load_wav_file warning: sample rate wrong, resampling from ", samplerate, "to", want_samplerate)
        data = sf.resample(data, target_samplerate=want_samplerate)

    # If the file has more than one channel, only return the first channel
    if len(data.shape) > 1 and data.shape[1] > 1:
        data = data[:, 0]

    # Put each sample in its own array
    # so we have [[sample1], [sample2]]
    data = normalize(np.array(data))
    data = data[:, np.newaxis]

    return data

def audio_file_to_fragments(audio_filepath, frag_len_seconds, samplerate):
    """
    load in the sent audio file and chop it into fragments of the sent length
    """
    assert os.path.exists(audio_filepath), "Cannot find audio file " + audio_filepath
    #audio_data, sr = sf.read(audio_filepath)
    
    audio_data = load_wav_file(audio_filepath, samplerate)
    num_samples_per_frag = 2048 #int(frag_len_seconds * samplerate)
    num_frags = int(np.ceil(len(audio_data) / num_samples_per_frag))

    fragments = []
    for i in range(num_frags):
        frag_start = i * num_samples_per_frag
        frag_end = (i + 1) * num_samples_per_frag

        
        fragment = audio_data[frag_start:frag_end]

        if len(fragment) != num_samples_per_frag:
            continue 
        fragments.append(fragment)

    return np.array(fragments, dtype=np.single)

def audio_filelist_to_fragments(audio_files, frag_len_seconds, samplerate):
    """
    iterates the sent list of audio files
    loads their data, chops to fragments
    returns a list of all the fragments
    """
    all_fragments = []
    for file in audio_files:
        fragments = audio_file_to_fragments(file, frag_len_seconds, samplerate)
        all_fragments.extend(fragments)
        
    return np.array(all_fragments)

def normalize(data):
    data_max = max(data)
    data_min = min(data)
    data_norm = max(data_max,abs(data_min))
    return data / data_norm

def generate_dataset(input_audio_folder, output_audio_folder, frag_len_seconds=0.5, samplerate=44100):#, pre_emphasis_filter=True):
    """
    Generates the complete dataset from which 
    training, validation and testing data will be retrieved
    Wright used half second segments for each data point
    returns a  TensorDataset object suitable for use with Torches dataloader: 
    dataloader = DataLoader(dataset, batch_size=32, shuffle=True)
    """
    assert os.path.exists(input_audio_folder), "Input audio folder not found " + input_audio_folder
    assert os.path.exists(output_audio_folder), "Output audio folder not found " + output_audio_folder

    # get audio files in the input folder
    input_files = get_files_in_folder(input_audio_folder, ".wav")
    output_files = get_files_in_folder(output_audio_folder, ".wav")
    assert len(input_files) > 0, "get_files_in_folder yielded zero inputs files"
    assert len(output_files) > 0, "get_files_in_folder yielded zero outputs files"

    # naming conventions:
    #   - input:    name-input.wav
    #   - output:   name-target.wav

    # dictionarization
    # name: filename
    input_dict = {input_file.split("/")[-1].split("-")[0]: input_file for input_file in input_files}
    
    check_input_naming(input_dict)

    init_arrays = True # control val
    np_array_input = 0
    np_array_output = 0

    for name in input_dict:
        print(f"loading input and output of {name}")

        # dictionarization
        # name: filename
        output_dict = {
            output_file_name.split("/")[-1].split("-")[0]: output_file_name 
                for output_file_name in output_files 
                if check_output_naming(output_file_name, name)
        }
        
        assert len(output_dict) == len(input_dict), f"found a number of output != 1: {name}"

        print(f"    loading output of {name}")

        input_fragments = audio_filelist_to_fragments([input_dict[name]], frag_len_seconds, samplerate)
        output_fragments = audio_filelist_to_fragments([output_dict[name]], frag_len_seconds, samplerate)

        # something went terribly wrong, should never fire
        assert len(input_fragments) > 0, "get_files_in_folder yielded zero inputs"
        assert len(output_fragments) > 0, "get_files_in_folder yielded zero outputs"
        
        # make lengths the same
        if len(input_fragments) > len(output_fragments):
            input_fragments = input_fragments[0:len(output_fragments)]
        else:
            output_fragments = output_fragments[0:len(input_fragments)]
        print("    generate_dataset:: Loaded frames from audio file", len(input_fragments[0]))
        # Convert input and output fragments to PyTorch tensors
        # noting that the normal shape for an input to an LSTM 
        # is (sequence_length, batch_size, input_size]
        # so input_fragments[0]

        print(f"    found input fragments of shape {input_fragments.shape}")

        if init_arrays:
            np_array_input = np.array(input_fragments)
        else:
            np_array_input = np.concatenate((np_array_input, np.array(input_fragments)))
        

        print(f"    found output fragments of shape {output_fragments.shape}")

        if init_arrays:
            np_array_output = np.array(output_fragments)
            init_arrays = False
        else:
            np_array_output = np.concatenate((np_array_output, np.array(output_fragments)))
        
        print(f"    total input shape: {np_array_input.shape}")
        print(f"    total output shape: {np_array_output.shape}\n")     
        
    # input_tensor = torch.tensor(np_array_input)
    # output_tensor = torch.tensor(np_array_output) 
    # print("total input tensor shape", input_tensor.shape)
    # print("total output tensor shape",output_tensor.shape)
    # dataset = TensorDataset(input_tensor, output_tensor)

    # np_array_input = np_array_input.transpose((0,2,1))
    # np_array_output = np_array_output.transpose((0,2,1))
    
    x_train, x_val, y_train, y_val = train_test_split(np_array_input, np_array_output, test_size=0.20, random_state=42)   

    train_dataset = tf.data.Dataset.from_tensor_slices((x_train, y_train))
    val_dataset = tf.data.Dataset.from_tensor_slices((x_val, y_val))
    
    # train_samples = train_dataset._tensors[0].shape[0]
    # val_samples = val_dataset._tensors[0].shape[0]

    # max_divider = gcd(train_samples, val_samples)
    # batch_size_train = train_samples / min_divider
    # batch_size_val = val_samples / min_divider
    
    # print(f"dividing in {min_divider}, num samples per sequence {batch_size_train}, {batch_size_val}")

    # train_dataset = train_dataset.batch(batch_size_train)
    # val_dataset = val_dataset.batch(batch_size_val)

    return [train_dataset, val_dataset]
    

def check_input_naming(input_dict):
    # 2 fields, the 2° is "input.wav"
    for name in input_dict:
        split = input_dict[name].split("/")[-1].split("-")
        assert len(split) == 2 and split[1] == "input.wav", "incorrect input naming convention, please name the inputs: name-input.wav"

def check_output_naming(output_file_path, name):
    output_file_name = output_file_path.split("/")[-1]
    split = output_file_name.split("-")

    # check structure and name
    assert len(split) == 2, "incorrect target naming convention, please name the outputs: name-target.wav"

    if split[0] == name:
        # check target keyward
        assert output_file_name == f"{name}-target.wav", "incorrect target naming convention, please name the outputs: name-target.wav"

        return True
    
    return False 