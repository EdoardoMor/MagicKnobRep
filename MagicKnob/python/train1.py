## This script trains an LSTM according
## to the method described in 
## A. Wright, E.-P. Damskägg, and V. Välimäki, ‘Real-time black-box modelling with recurrent neural networks’, in 22nd international conference on digital audio effects (DAFx-19), 2019, pp. 1–8.
import datetime
import myk_data_conv
import myk_models
import myk_loss 
import myk_train
import torch 
from torch.utils.data import DataLoader
import myk_evaluate_conv
import os 

from torch.utils.tensorboard import SummaryWriter

# used for the writing of example outputs
run_name="audio_conv1D"
# dataset : need an input and output folder in this folder
audio_folder = "./MagicKnob/data/audio_conv1D"
#audio_folder = "../../data/audio_ht1"
assert os.path.exists(audio_folder), f"Audio folder  not found. Looked for {audio_folder}"
# used to render example output during training
test_file = "./MagicKnob/data/guitar.wav"
assert os.path.exists(test_file), "Test file not found. Looked for " + test_file

lstm_hidden_size = 32
learning_rate = 5e-3
batch_size = 50
max_epochs = 10000

# create the logger for tensorboard
writer = SummaryWriter()

print("Loading dataset from folder ", audio_folder)
dataset = myk_data_conv.generate_dataset(audio_folder + "/input/", audio_folder + "/output/", frag_len_seconds=0.5)

print("Splitting dataset")
train_ds, val_ds, test_ds = myk_data_conv.get_train_valid_test_datasets(dataset)

print("Looking for GPU power")
device = myk_train.get_device()

print("Creating model")
model = myk_models.SimpleConv1D().to(device)

print("Creating data loaders")
train_dl = DataLoader(train_ds, batch_size=batch_size, shuffle=True, generator=torch.Generator(device=device))
val_dl = DataLoader(val_ds, batch_size=batch_size, shuffle=True, generator=torch.Generator(device=device))
test_dl = DataLoader(test_ds, batch_size=batch_size, shuffle=True, generator=torch.Generator(device=device))

print("Creating optimiser")
# https://github.com/Alec-Wright/Automated-GuitarAmpModelling/blob/main/dist_model_recnet.py
optimiser = torch.optim.Adam(model.parameters(), lr=learning_rate, weight_decay=1e-4)
scheduler = torch.optim.lr_scheduler.ReduceLROnPlateau(optimiser, 'min', factor=0.5, patience=5, verbose=True)
print("Creating loss functions")
# https://github.com/Alec-Wright/CoreAudioML/blob/bad9469f94a2fa63a50d70ff75f5eff2208ba03f/training.py
loss_functions = myk_loss.LossWrapper()
# now the training loop

print("About to train")
lowest_val_loss = 0
best_loss = False

max_patience = 500
curr_patience = max_patience

# datetime object containing current date and time
now = datetime.datetime.now()
dt_string = now.strftime("Conv2D_%d-%m-%Y_%H:%M:%S")

os.mkdir(os.path.join(".", dt_string))

for epoch in range(max_epochs):
    ep_loss = myk_train.train_epoch_conv(model, train_dl, loss_functions, optimiser, device=device)
    
    #ep_loss = myk_train.train_epoch(model, train_dl, loss_functions, optimiser, device=device)
    val_loss = myk_train.compute_batch_loss(model, val_dl, loss_functions, device=device)
    writer.add_scalar("Loss/val", val_loss, epoch)
    writer.add_scalar("Loss/train", ep_loss, epoch)
    
    # check if we have beaten our best loss to date
    if lowest_val_loss == 0:# first run
        lowest_val_loss = val_loss
    elif val_loss < lowest_val_loss:# new record
        lowest_val_loss = val_loss
        best_loss = True
    else: # no improvement
        best_loss = False
        curr_patience -= 1

    if best_loss: # save best model so far
        print(f"   Record loss - saving at epoch {epoch}")
        model.save_for_rtneural(f"{dt_string}/model.json")
        print(f"   epoch {epoch}, train_loss {ep_loss}, val_loss {val_loss} ")
        curr_patience = max_patience
        myk_evaluate_conv.run_file_through_model(model, test_file, audio_folder + "/" + run_name + str(epoch)+".wav")
    if epoch % 50 == 0: # save an example processed audio file
        #myk_evaluate_conv.run_file_through_model(model, test_file, audio_folder + "/" + run_name + str(epoch)+".wav")
        print(f"epoch {epoch}, train_loss {ep_loss}, val_loss {val_loss} ")
    if curr_patience == 0:
        print("max patience reached, stopping")
        break