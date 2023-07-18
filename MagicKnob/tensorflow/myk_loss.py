# pretty much verbatim
# from here: 
# https://github.com/Alec-Wright/CoreAudioML/blob/bad9469f94a2fa63a50d70ff75f5eff2208ba03f/training.py

import tensorflow as tf 

# ESR loss calculates the Error-to-signal between the output/target
class ESRLoss(tf.keras.losses.Loss):
    def __init__(self):
        super(ESRLoss, self).__init__()
        self.epsilon = 0.00001

    def call(self, output, target):
        loss = tf.add(target, -output)
        loss = tf.pow(loss, 2)
        loss = tf.reduce_mean(loss)
        energy = tf.reduce_mean(tf.pow(target, 2)) + self.epsilon
        loss = tf.divide(loss, energy)
        return loss


class DCLoss(tf.keras.losses.Loss):
    def __init__(self):
        super(DCLoss, self).__init__()
        self.epsilon = 0.00001

    def call(self, output, target):
        loss = tf.pow(tf.add(tf.reduce_mean(target, 0), -tf.reduce_mean(output, 0)), 2)
        loss = tf.reduce_mean(loss)
        energy = tf.reduce_mean(tf.pow(target, 2)) + self.epsilon
        loss = tf.divide(loss, energy)
        return loss

class LossWrapper(tf.keras.losses.Loss):
    def __init__(self, losses={'ESR': 0.75, 'DC': 0.25}):
        """
        losses: list of names of loss functions to apply
        """
        super(LossWrapper, self).__init__()
        loss_dict = {'ESR': ESRLoss(), 'DC': DCLoss()}
        loss_functions = [[loss_dict[key], value] for key, value in losses.items()]

        self.loss_functions = tuple([items[0] for items in loss_functions])
        try:
            self.loss_factors = tuple([items[1] for items in loss_functions])
        except IndexError:
            self.loss_factors = tf.ones(len(self.loss_functions))

    def call(self, target, output):
        loss = 0
        for i, losses in enumerate(self.loss_functions):
            loss += tf.multiply(losses(output, target), self.loss_factors[i])
        return loss


class TrainTrack(dict):
    def __init__(self):
        self.update({'current_epoch': 0, 'training_losses': [], 'validation_losses': [], 'train_av_time': 0.0,
                     'val_av_time': 0.0, 'total_time': 0.0, 'best_val_loss': 1e12, 'test_loss': 0})

    def restore_data(self, training_info):
        self.update(training_info)

    def train_epoch_update(self, loss, ep_st_time, ep_end_time, init_time, current_ep):
        if self['train_av_time']:
            self['train_av_time'] = (self['train_av_time'] + ep_end_time - ep_st_time) / 2
        else:
            self['train_av_time'] = ep_end_time - ep_st_time
        self['training_losses'].append(loss)
        self['current_epoch'] = current_ep
        self['total_time'] += ((init_time + ep_end_time - ep_st_time)/3600)

    def val_epoch_update(self, loss, ep_st_time, ep_end_time):
        if self['val_av_time']:
            self['val_av_time'] = (self['val_av_time'] + ep_end_time - ep_st_time) / 2
        else:
            self['val_av_time'] = ep_end_time - ep_st_time
        self['validation_losses'].append(loss)
        if loss < self['best_val_loss']:
            self['best_val_loss'] = loss