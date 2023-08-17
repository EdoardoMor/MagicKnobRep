## A. Wright, E.-P. Damskägg, and V. Välimäki, ‘Real-time black-box modelling with recurrent neural networks’, in 22nd international conference on digital audio effects (DAFx-19), 2019, pp. 1–8.

import torch 
import json
from json import JSONEncoder

import torch.nn.functional as F

class SimpleLSTM(torch.nn.Module):
    """
    LSTM Model after
    A. Wright, E.-P. Damskägg, and V. Välimäki, ‘Real-time black-box modelling with recurrent neural networks’, in 22nd international conference on digital audio effects (DAFx-19), 2019, pp. 1–8.
    uses 32 hidden by default.
    Wright et al. showed decent performance for 32, but 
    even better going up to 96
    """

    model_type = "LSTM"
    input_dim = 1

    def __init__(self, hidden_size=32, num_layers=1, dropout=0, param=False): 
        super().__init__()
        # Batch first means input data is [batch,sequence,feature]

        if param:
            self.input_dim = 2

        self.lstm = torch.nn.LSTM(input_size=self.input_dim, hidden_size=hidden_size, batch_first=True,
                                  num_layers=num_layers, dropout=dropout)
        self.dense = torch.nn.Linear(hidden_size, 1)# from 8 hidden back to 1 output
        self.drop_hidden = True


    def zero_on_next_forward(self):
        """
        next time forward is called, the network will
        run it with zeroed hidden+cell values
        """
        self.drop_hidden = True 
    
    def forward(self, torch_in):
        if self.drop_hidden:
            batch_size = torch_in.shape[0]
            # h_size = (num_layers,batch_size,hidden_count)
            h_shape = [self.lstm.num_layers, batch_size, self.lstm.hidden_size]
            #print("dropping hidden, shape probably ", h_shape)
            hidden = torch.zeros(h_shape).to(torch_in.device)
            cell = torch.zeros(h_shape).to(torch_in.device)
            x, _ = self.lstm(torch_in, (hidden, cell))
            self.drop_hidden = False
        else:
            x, _ = self.lstm(torch_in)
            x += torch_in

        return self.dense(x)
    
    def save_for_rtneural(self, outfile):
        ## used for saving 
        class EncodeTensor(JSONEncoder):
            def default(self, obj):
                if isinstance(obj, torch.Tensor):
                    return obj.cpu().detach().numpy().tolist()
                return super(json.NpEncoder, self).default(obj)
            
        with open(outfile, 'w') as json_file:
            json.dump(self.state_dict(), json_file,cls=EncodeTensor)

# -------------------------------------------------------------------------------------------------------- #

class SimpleConv1D(torch.nn.Module):

    model_type = "Conv1D"
    input_dim = 1

    def __init__(self, param=False):
        super().__init__()

        if param:
            self.input_dim = 2

        self.conv1 = torch.nn.Conv1d(self.input_dim, 16, kernel_size=5, stride=1, padding='same')
        self.act1 = torch.nn.PReLU()

        self.conv2 = torch.nn.Conv1d(16, 32, kernel_size=5, stride=1, padding='same')
        self.act2 = torch.nn.PReLU()

        # commented for dist

        self.conv3 = torch.nn.Conv1d(32, 64, kernel_size=5, stride=1, padding='same')
        self.act3 = torch.nn.PReLU()

        self.conv4 = torch.nn.Conv1d(64, 32, kernel_size=5, stride=1, padding='same')
        self.act4 = torch.nn.PReLU()

        # till here

        self.conv5 = torch.nn.Conv1d(32, 16, kernel_size=5, stride=1, padding='same')
        self.act5 = torch.nn.PReLU()

        self.conv6 = torch.nn.Conv1d(16, 1, kernel_size=5, stride=1, padding='same')
        self.act6 = torch.nn.PReLU()
 
    def forward(self, x):
        #print("in forward\ninput shape: " + str(x.size()))
        # x, phase = self.batch_stft(x)
        #print("input shape post stft: " + str(x.size()))
 
        x = x.transpose(-2, -1)
        #print(f"shape after transp: {x.size()}")

        #print("input shape after transpose: " + str(x.size()))

        # input 1xFxT, output 16xFxT
        x = self.act1(self.conv1(x))
        # input 16 channels, output 32 channels
        x = self.act2(self.conv2(x))

        # input 32 channels, output 64 channels
        x = self.act3(self.conv3(x))

        # input 64 channels, output 32 channels
        x = self.act4(self.conv4(x))

        # input 32 channels, output 16 channels
        x = self.act5(self.conv5(x))

        # input 16 channels, output 1 channel
        x = self.act6(self.conv6(x))


        x = x.transpose(-2, -1)

        #print("output shape: " + str(x.size()))

        return x

    def save_for_rtneural(self, outfile):
        ## used for saving 
        class EncodeTensor(JSONEncoder):
            def default(self, obj):
                if isinstance(obj, torch.Tensor):
                    return obj.cpu().detach().numpy().tolist()
                return super(json.NpEncoder, self).default(obj)
            
        with open(outfile, 'w') as json_file:
            json.dump(self.state_dict(), json_file,cls=EncodeTensor)

# -------------------------------------------------------------------------------------------------------- #

class SimpleConv2D(torch.nn.Module):

    model_type = "Conv2D"
    input_dim = 1

    def __init__(self, param=False):
        super().__init__()

        if param:
            self.input_dim = 2

        self.conv1 = torch.nn.Conv2d(self.input_dim, 16, kernel_size=(3,3), stride=1, padding='same')
        self.act1 = torch.nn.ReLU()

        self.conv2 = torch.nn.Conv2d(16, 32, kernel_size=(3,3), stride=1, padding='same')
        self.act2 = torch.nn.ReLU()

        self.conv3 = torch.nn.Conv2d(32, 64, kernel_size=(3,3), stride=1, padding='same')
        self.act3 = torch.nn.ReLU()

        self.conv4 = torch.nn.Conv2d(64, 32, kernel_size=(3,3), stride=1, padding='same')
        self.act4 = torch.nn.ReLU()

        self.conv5 = torch.nn.Conv2d(32, 16, kernel_size=(3,3), stride=1, padding='same')
        self.act5 = torch.nn.ReLU()

        self.conv6 = torch.nn.Conv2d(16, 1, kernel_size=(3,3), stride=1, padding='same')
        self.act6 = torch.nn.ReLU()
 
    def forward(self, x):
                
        # print("input shape: " + str(x.size()))
        x, phase = self.batch_stft(x)
        #print("input shape post stft: " + str(x.size()))

        x = x.transpose(0, 1)

        #print("input shape after transpose: " + str(x.size()))

        # input 1xFxT, output 16xFxT
        x = self.act1(self.conv1(x))
        # input 16 channels, output 32 channels
        x = self.act2(self.conv2(x))

        # input 32 channels, output 64 channels
        x = self.act3(self.conv3(x))

        # input 64 channels, output 32 channels
        x = self.act4(self.conv4(x))

        # input 32 channels, output 16 channels
        x = self.act5(self.conv5(x))

        # input 16 channels, output 1 channel
        x = self.act6(self.conv6(x))

        x = x.transpose(0, 1)


        #print("output shape pre istft: " + str(x.size()))
        x = self.batch_istft(x, phase, trim_length=None)

        x = torch.unsqueeze(x, 2)
        # print("output shape post istft: " + str(x.size()))

        return x
    
    def batch_stft(self, x, pad: bool = True, return_complex: bool = False):

        x = torch.squeeze(x)
        #print("x shape after squeeze: " + str(x.size()))


        x = x.reshape(-1, x.size()[-1])

        #print("x shape after reshaping: " + str(x.size()))
        

        #if pad:
            #x = self.pad_stft_input(x)

        #print("x sizes after padding: " + str(x.size()))
            

        S = self._stft(x)

        #print("S shape: " + str(S.shape[:]))

        #print("x_shape[:-1]: " + str(x_shape[:-1]))

        #print("S.shape[-2:]: " + str(S.shape[-2:]))

        #S = S.reshape(x_shape[:-1] + S.shape[-2:]) # aggiunge una dimensione alla S: da [2, 2049, 1347] -> [1, 2, 2049, 1347]

        S = torch.unsqueeze(S, 0)

        #print("S shape(), post unsqueeze: " + str(S.shape[:]))

        if return_complex:
            return S
        return S.abs(), S.angle()

    def batch_istft(self, magnitude, phase, trim_length=None):
        S = torch.polar(magnitude, phase)
        #S = S.reshape(-1, S.size()[-2], S.size()[-1])
        S = torch.squeeze(S, 0)
        #print("Nell' istft S shape: " + str(S.size()))
        y = self._istft(S, trim_length)
        #x = x.reshape(S_shape[:-2] + x.shape[-1:])
        y_shape = y.size()
        #print("Nell'istft y shape: " + str(y_shape))
        return y

    def _stft(self, x):
        return torch.stft(input=x,
                          n_fft=4096,
                          window=torch.hann_window(4096, periodic=True),#.to(device),
                          win_length=4096,
                          hop_length=4096//8,
                          center=True,
                          return_complex=True
                          )

    def _istft(self, x, trim_length=None):
        return torch.istft(input=x,
                           n_fft=4096,
                           window=torch.hann_window(4096, periodic=True),#to.(device)
                           win_length=4096,
                           hop_length=4096//8,
                           center=True,
                           length=trim_length
                           )
    
    def pad_stft_input(self, x):
        pad_len = (-(x.size(-1) - 4096) % (4096//4)) % 4096
        return F.pad(x, (0, pad_len*4))

    def save_for_rtneural(self, outfile):
        ## used for saving 
        class EncodeTensor(JSONEncoder):
            def default(self, obj):
                if isinstance(obj, torch.Tensor):
                    return obj.cpu().detach().numpy().tolist()
                return super(json.NpEncoder, self).default(obj)
            
        with open(outfile, 'w') as json_file:
            json.dump(self.state_dict(), json_file,cls=EncodeTensor)