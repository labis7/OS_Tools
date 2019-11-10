#Python interpreter doesn't allocate mem for all the variables
# --> if two or more variable has the same value it just map to that value
#So we can use the average complexity O(1) of the dictionary to search fast products and afm
def add_to_dictionary(rec_t, mydix, counter):
    mydix[rec_t[0]] = counter
    for i in range(1, len(rec_t)-1, 4):
        try:
            mydix[rec_t[i]].append(counter)
        except:
            #make a new list for the specific product
            #each entry in the list is a index to a tuple(afm,product1...) in mylist
            mydix[rec_t[i]] = [counter]


def split_and_tuple(file, final_list, mydix):
    f = open(file)
    final_list=[]
    receipt=[]
    for line in f:          ## For loop on each line of the file
        if(line[0] == "-"): # continue to the next usefull line(New receipt)
            if(len(receipt) > 0):    #Check if last temp_list has usefull data(it doesnt in case of the 1st receipt)
                receipt.pop(len(receipt)-2) # Pop the SUM WORD from the list
                receipt_t=tuple(receipt[1:len(receipt)]) # Tuple of a receipt is ready(Without the AFM word)
                final_list.append(receipt_t)  ##Added to the final list of tuples

                #####
                #Check Dictionary,add the restaurant(afm) and products so they map to the final_list index ,
                # if exists you will get the number from dix in order to co-sum(NOT SURE IF THAT IS POSSIBLE YET)
                add_to_dictionary(receipt_t, mydix, len(final_list)-1)

                #####
            receipt=[]      #New temp_list(for our new receipt)
            continue        ## No other words in the line
        for word in line.split(): ##Splitting each Line into words #   Tip:  .SPLIT won't split 'Product:' ,(No pop yet)
            receipt.append(word)
    f.close()
    return final_list,mydix



my_input=-1
mydix={}
mylist=[]
while(my_input != 4):
    my_input=input("Give your preference: (1: read new input file, 2: print statistics for a specific product, 3: print statistics for a specific AFM, 4: exit the program)")
    ########### Exception Handling ############
    #not yet handled
    my_input=int(my_input)
    ###########################################

    if(my_input == 1):
        file = input("Give the name of the file:")

        mylist,mydix=split_and_tuple(file, mylist, mydix) #a list of tuples is returned,
        print(mylist)
        print(mydix)
        # format of each touple:
        # (afm_num,prod_name1,quantity1,price_per_prod1,sum_of_prod1,prod_nameN,quantityN,price_per_prodN,sum_of_prodN,SUM)
    elif(my_input == 2):  ## Statistics according to a product
        continue
    elif(my_input == 3): ## Statistics according to AFM
        continue
    elif(my_input == 4):
        exit(0)


