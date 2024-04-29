#! /usr/bin/env python3
import torch
from torch import nn
from torch.utils.data import Dataset, DataLoader
from torch.nn import functional as F 

from sklearn import metrics 

import pandas as pd 
import matplotlib.pyplot as plt
import re
from tqdm import tqdm
import numpy as np
import csv
import json

import os 

headers = ['raw_string', 'loop_depth', 'number_BB', 'number_exits', 'number_exit_blocks', 'num_successors', 'isexit', 'isbackedge', 'isdestinationinloop', 'isdestinationnestedloop', 'opcode_condition', 'prev_instr_1', 'prev_instr_2', 'operand_1.constant', 'operand_1.isfunctionarg', 'operand_1.isglobalvar', 'operand_1.isgeneralvar', 'operand_1.other', 'operand_2.constant', 'operand_2.isfunctionarg', 'operand_2.isglobalvar', 'operand_2.isgeneralvar', 'operand_2.other', 'operand_3.constant', 'operand_3.isfunctionarg', 'operand_3.isglobalvar', 'operand_3.isgeneralvar', 'operand_3.other', 'operand_4.constant', 'operand_4.isfunctionarg', 'operand_4.isglobalvar', 'operand_4.isgeneralvar', 'operand_4.other', 'branch_prob']


class LinearRegressionFFN(nn.Module):
    def __init__(self, input_dim, hidden_dim=50):
        super(LinearRegressionFFN, self).__init__()
        self.linear1 = nn.Linear(input_dim, hidden_dim)
        self.linear2 = nn.Linear(hidden_dim, 1)

    def forward(self, input):
        hidden_output = F.relu(self.linear1(input), dim=-1)
        return torch.sigmoid(self.linear2(hidden_output))
    

class FFNN(nn.Module): 
    def __init__(self,input_dim, hidden_dim=50,n_layers=3): 
        super().__init__() 
        dim = input_dim 
        layers = [] 
        for i in range(n_layers-1):
            layers.append(
                nn.Linear(dim,hidden_dim)
            )
            dim = hidden_dim
        layers.append(nn.Linear(dim,1))

        self.layers = nn.ModuleList(layers)
        self.double()

    def forward(self,x): 
        for layer in self.layers[:-1]: 
            x = F.relu(layer(x))
        return F.sigmoid(self.layers[-1](x))



def tabulate_files(target_dir,split={'train':0.7,'val':0.15,'test':0.15},seed=42,min_lines=10,downsample_fac=0.0):


    rg = np.random.default_rng(seed=seed)


    rows = []
    for statfile_name in tqdm(os.scandir(target_dir),total=len(list(os.scandir(target_dir)))): 
        match:re.Match = re.match(r'p(\d+)_s(\d+)\.opcstats',statfile_name.name)
        if match is not None: 
            task_id,submission_id = match.groups()
            
            header = None 
            with open(statfile_name.path,'r') as file: 
                if re.findall(r'error|warning|(PLEASE submit a bug report)|abort|(core dumped)|(timeout --preserve-status)',file.read()): 
                    if  'timeout --preserve-status' in file: 
                        header = 1 
                    else: 
                        continue
            try:
                csv = pd.read_csv(statfile_name.path, header=header, names=headers,delimiter=',')
                correct = True
            except pd.errors.ParserError: 
                correct=False
            
            if correct and len(csv) > min_lines  and rg.random() >= downsample_fac:
                rows.append({'task':task_id,'submission':submission_id, 'n_rows':len(csv),'path':statfile_name.path})

    files_df  = pd.DataFrame(rows).dropna()
    n_lines = files_df['n_rows'].sum() 

    tasks = files_df['task'].unique()
    rg.shuffle(tasks)
    files_df['fold'] = None

    split_it = iter(split.items())
    split_n, split_perc = next(split_it)
    fold_lines = 0 
    for task in tasks: 
        # if fold_lines < 30: 
        #     print(split_n, task,fold_lines)
        
        if fold_lines >= split_perc * n_lines: 
            split_n, split_perc = next(split_it)
            fold_lines = 0 

        files_df.loc[files_df['task']== task,'fold'] = split_n
        fold_lines += files_df.loc[files_df['task']== task,'n_rows'].sum()

    return files_df
            

def load_file(file_path): 
    curr_file_df:pd.DataFrame  = pd.read_csv(file_path,header=None, names=headers).drop(columns=['raw_string']) 
    # print(curr_file_df)
    probs = curr_file_df['branch_prob'].to_numpy()
    data = curr_file_df.drop(columns=['branch_prob']).to_numpy()
    if np.isnan(data).any() or np.isnan(probs).any(): 
        print(probs, data)
        with open(file_path,'r') as file: 
            print(file.read())
        raise TypeError(f'Error with loading data point for file {file}')
    return probs,data
        


class InMemDataset(Dataset): 
    """
    in Memory for our training script
    
    """
    def __init__(self, fold, fold_df:pd.DataFrame): 
        super().__init__()

        files = fold_df[fold_df['fold'] == fold]['path'].to_list()
        datas = []
        self.probs = []
        for file in tqdm(files): 
            fprob, fdat = load_file(file)
            # print(fprob)
            # print(fdat)
            # print(fprob.dtypes)
            # print(fdat.dtype)


            self.probs.extend(fprob)
            datas.append(torch.tensor(fdat,dtype=torch.float64))

            # if np.isnan(fprob).any() or np.isnan(fdat).any(): 
            #     print(fprob, fdat)
            #     with open(file,'r') as file: 
            #         print(file.read())
            #     break


            
        
        self.data = torch.cat(datas,dim=0)

    
    def __getitem__(self,i): 
        # print(i)
        return  self.data[i], self.probs[i]
    
    def __len__(self):
        return len(self.probs)
    
     
class OomDataset(Dataset): 
    """
    in Memory for our training script
    
    """
    def __init__(self, fold, fold_df:pd.DataFrame): 
        super().__init__()

        self.files = fold_df[fold_df['fold'] == fold,'path'].to_list()

    
    class iter : 
        def __init__(self,dataset): 
            self.ds = dataset
            self.file_no = 0 
            self.line_no = 0 

            self.file_data = None
            self.file_probs = None

        def load_next_file(self): 
            self.file_no +=1 
            self.line_no = 0 

            fp, fd = load_file(self.ds.files[self.file_no])
            self.file_probs = torch.tensor(fp)
            self.file_data = torch.tensor(fd)


        def __next__(self): 
            if self.file_data is None or self.file_probs is None or self.line_no >=  len(self.file_probs):
                self.load_next_file()
            self.line_no +=1 
            return self.file_data[self.line_no -1 ], self.file_probs[self.line_no -1 ]
        
    def __iter__(self): 
        return self._iter(self)
    

def get_dataloaders(target_dir, split={'train':0.7,'val':0.15,'test':0.15},seed=42,min_lines=10,batch_size=16,n_workers=8,downsample_by=0.0, **kwargs): 
    data_index =  tabulate_files(target_dir,split=split,seed=seed,min_lines=min_lines,downsample_fac=downsample_by)

    folds ={}
    for fold in split.keys(): 
        folds[fold] = DataLoader(
            InMemDataset(fold=fold,fold_df=data_index),
            batch_size=batch_size,
            num_workers=n_workers,
            shuffle=True,
            **kwargs
        )

    return folds,data_index


def train_model(train_dl,eval_dl,model:torch.nn.Module,device='cpu',n_epochs=5,lr=1e-3,eval_every=None,avg_over=20):
    
    model.train()
    model = model.to(device)

    loss_fn = torch.nn.MSELoss()
    optimizer = torch.optim.AdamW(model.parameters(), lr=lr)

    eval_every = eval_every or len(train_dl)//4
    
    losses = []
    val_losses = []
    losses_temp = []

    print(eval_every)

    for epoch in range(n_epochs): 
        pbar = tqdm(train_dl)
        for i,(data, labels) in enumerate(pbar): 
            optimizer.zero_grad()

            data = data.to(device)
            labels = labels.to(device)

            preds = model(data)
            loss = loss_fn(preds,labels[:,None])

            loss.backward()
            optimizer.step()

            # print(loss)
            # print(preds)
            # print(labels)
            losses_temp.append(float(loss.detach().cpu()))


            if i % eval_every == 0: 
                # print("\n\n\n\n~~~~~~~~~~~~~~~~~~~~~~EVAL~~~~~~~~~~~~~~~~~~~~~")
                old_desc = pbar.desc
                pbar.desc = f'{old_desc} [EVALUATING]'
                pbar.refresh()

                with torch.no_grad(): 
                    vloss = 0 
                    n_examples = 0 
                    model.eval()
                    for val_data, val_labels in eval_dl: 
                        val_data = val_data.to(device) 
                        val_labels = val_labels.to(device) 

                        val_pred = model(val_data)
                        # print(val_labels.shape, val_data.shape,val_pred.shape)
                        vloss += float(loss_fn(val_pred,val_labels[:,None]).detach().cpu())*len(val_labels)
                        n_examples += len(val_labels)

                    model.train()
                    vloss /= n_examples
                    val_losses.append(vloss)
                pbar.desc = old_desc 

            if len(losses_temp) == avg_over: 
                losses.append(sum(losses_temp)/avg_over)
                losses_temp = []
                pbar.desc = f'epoch:{epoch +1} loss: {losses[-1]:1.4f}'
    return model, losses, val_losses

def test_model(model, dataloader,device='cpu',metric_funcs = {'MSE':metrics.mean_squared_error,'MAE':metrics.mean_absolute_error}): 
    loss_fn = F.mse_loss
    labels = []
    preds = []

    with torch.no_grad(): 
        model.eval()
        for iter_data, iter_labels in dataloader: 
            iter_data:torch.Tensor = iter_data.to(device) 

            iter_preds:torch.Tensor = model(iter_data)
            # print(val_labels.shape, val_data.shape,val_pred.shape)
            preds.extend(list(iter_preds.cpu().detach().numpy().flatten()))
            labels.extend(list(iter_labels.cpu().detach().numpy()))

    results = {}
    for metric, func in metric_funcs.items(): 
        results[metric] = func(labels,preds)
    
    return labels, preds, results
    

# def test(): 

#     test_file = "../model_data/branch_test.csv"
#     branch_test_data = pd.read_csv(test_file)

#     branch_prob = 'branch_prob'
#     raw_string = 'raw_string'

#     clean_test_data = branch_test_data.loc[:, ~branch_test_data.columns.isin([branch_prob, raw_string])]
#     clean_test_labels = branch_test_data[branch_prob].tolist()

#     test_data = [(torch.Tensor(clean_test_data.iloc[i].tolist()), torch.Tensor([clean_test_labels[i]])) for i in range(len(clean_test_data))]


#     model.eval()

#     predictions = []

#     with torch.no_grad():
#         for step, data in enumerate(tqdm(test_data)):
#             inputs, label = data
#             prediction = model(inputs)
#             predictions.extend(prediction)



# from sklearn.metrics import mean_squared_error, mean_absolute_error

# print(f"RMSE for Test Set: {mean_squared_error(clean_test_labels, predictions, squared=False)}") #RMSE
# print(f"MAE for Test Set: {mean_absolute_error(clean_test_labels, predictions)}")



# predictions_file = "pytorch_test_predictions.csv"

# test_predictions = [predictions[i].item() for i in range(len(predictions))]

# test_predictions = pd.Series(test_predictions)

# test_predictions.to_csv(predictions_file, index=False, header=False)



# pytorch_file = "pytorch_branch_model.pt"

# torch.save(model.state_dict(), pytorch_file)

def run_hp_sweep(n_attempts,loaders, n_epochs = 6, seed=42,hidden_dim_range=(10,100),depth_range= (3,9),log_lr_range= (-4.5,-2.5)): 
    
    best_model = None 
    best_losses = None
    best_val_losses = None 
    best_params = None 

    best_metric = 1e6 

    rgen = np.random.default_rng(seed)


    for i in range(n_attempts): 
        
        h_dim = int(rgen.integers(hidden_dim_range[0],high=hidden_dim_range[1],endpoint=True))
        n_layers = int(rgen.integers(depth_range[0],high=depth_range[1],endpoint=True))

        # lr, we sale logartihmically... 
        rand = rgen.random()
        learning_rate = float(10**((log_lr_range[1]-log_lr_range[0]) * rand +log_lr_range[0]))

        print("Training with", h_dim,n_layers,learning_rate)
        model = FFNN(32,hidden_dim=h_dim, n_layers=n_layers)

        model, losses, val_losses = train_model(loaders['train'],loaders['eval'],model,n_epochs=n_epochs,lr=learning_rate)

        if val_losses[-1] < best_metric: 
            best_model = model 
            best_losses= losses
            best_val_losses = val_losses
            best_params = {
                "h_dim": h_dim, 
                "n_layers": n_layers, 
                "lr": learning_rate,
            }
            best_metric = val_losses[-1]

            print(f'new best metric: {best_metric}')
            print(best_params)

    return best_model, best_losses, best_val_losses, best_params


def main(): 
    base_dir = 'model_prototypes/models_out'

    targ_dir = '/Dataset/group22/processed/more_cpp_test/results/'
    targ_dir = '/Dataset/group22/processed/more_cpp_2/results/'


    loaders, split_files_df = get_dataloaders(target_dir=targ_dir,split ={'train':0.7,'eval':0.15,'test':0.15},downsample_by=0.0)


    split_files_df.to_csv(f'{base_dir}/dataset_splits.csv')
    # for a in loaders['train']:
    #     if any(np.isnan(a[1])): 
    #         print(a) 
    #         break
    n_epochs = 10
    model, losses, val_losses, params = run_hp_sweep(16,loaders, n_epochs=n_epochs)
        
    losses_x_fac = n_epochs*1.0 / len(losses)
    val_losses_x_fac = (1.0*n_epochs)/len(val_losses)
    losses_x = [i* losses_x_fac  for i in range(len(losses))] 
    val_losses_x = [i* val_losses_x_fac  for i in range(len(val_losses))] 

    fig = plt.figure()
    ax = plt.subplot(111)
    ax.plot(losses_x,losses)
    ax.plot(val_losses_x,val_losses)
    ax.legend(['train loss','val loss'])
    plt.show()


    # test model
    labels, preds, metrics = test_model(model, loaders['test'])
    

    for i in range(100): 
        testfile = f'{base_dir}/best_model_feats{i}.json'
        if not os.path.isfile(testfile):
            fig.savefig(f'{base_dir}/loss_curves{i}.png')

            torch.save(model.state_dict(),f'{base_dir}/model_state_{i}.pt')
            with open(testfile,'w') as ofile: 
                params['losses'] = list(losses)
                params['val_losses'] = list(val_losses)

                params['test'] = {'labels':labels,'preds':preds}
                params['accs'] =  metrics
                json.dump(params,ofile)
            break



if __name__=="__main__": 
    main()


