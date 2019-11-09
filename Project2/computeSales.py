

print( "Give your preference: (1: read new input file, 2: print statistics for a specific product, 3: print statistics for a specific AFM, 4: exit the program)")


def split_and_tuple(file):
    f = open(file)
    final_list=[]
    for line in f:          ## For loop on each line of the file
        if(line[0] == "-"): # continue to the next usefull line(New receipt)
            if(len(receipt) > 0):    #Check if last temp_list has usefull data(it doesnt in case of the 1st receipt)
                receipt.pop(len(receipt)-2) # Pop the SUM WORD from the list
                receipt_t=tuple(receipt[1:len(receipt)]) # Tuple of a receipt is ready(Without the AFM word)
                final_list.append(receipt_t)  ##Added to the final list of tuples
                #####
                #Check Dictionary,add the restaurant(afm) with the final_list index ,
                # if exists you will get the number from dix in order to co-sum(NOT SURE IF THAT IS POSSIBLE YET)
                #####
            receipt=[]      #New temp_list(for our new receipt)
            continue        ## No other words in the line
        for word in line.split(): ##Splitting each Line into words
            if((type(word) == int)||(type(word) == float)):  #We do this in order to control products better,see below
                receipt.append(word)                #Numbers only
            else:
                receipt.append(word[:len(word)-1])  # .SPLIT won't split 'Product:' ,we dont need ':'



    f.close()
    return final_list


file = input("Give the name of the file:")

mylist=split_and_tuple(file) #a list of tuples is returned,
# format of each touple:
# (afm_num,prod_name1,quantity1,price_per_prod1,sum_of_prod1,prod_nameN,quantityN,price_per_prodN,sum_of_prodN,SUM)


