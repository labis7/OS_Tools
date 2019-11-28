##################################   1   #################################################################
import os ##For path input support

def validate_input(receipt):
    ## Check if the size of receipt is right(the right numbepath=os.getcwd()+filer of elements is 4 + n*4, for n=[1,+inf) )
    if(not ((len(receipt) - 4)/4).is_integer()): #IF its not INT, then ignore receipt
        return False
    sum = 0
    if (not ((receipt[len(receipt) - 2].casefold() == "συνολο:") and (receipt[0].casefold() == "αφμ:") and ((len(receipt[1]) == 10) and (receipt[1].isdigit())))): #afm name and number check
        return False
    for i in range(2, len(receipt)-2, 4): #Ignore 2 first and the 2 last elements of the receipt
        if(not (receipt[i + 1].isdigit())): #quantity check(Int value)
            return False
        if( not ((receipt[i + 2].replace(".","").isdigit()) and (receipt[i + 3].replace(".","").isdigit()))): #Price per element, sum for this element check
            return False
        if (round(int(receipt[i+1])*float(receipt[i+2]),2) == float(receipt[i+3])): #Check the subpart sum,if its right, add it ot the whole sum
            sum = sum + float(receipt[i+3])
        else:
            return False
    sum=round(sum,2)
    if( sum == float(receipt[len(receipt)-1])): #Check if  the Sum is right
        return True
    else:
        return False

#So we can use the average complexity O(1) of the dictionary to search fast products and afm
# (Worst case is O(n) but in rare cases where hash function is bad + when we have complicated structures in dix)
def add_to_dictionary(rec_t, mydix, counter):
    try:
        mydix[rec_t[0]]  #If exists,do nothing
    except:
        mydix[rec_t[0]] = counter
    for i in range(1, len(rec_t), 4):
        try:
            if(not(counter in mydix[rec_t[i].casefold()])):  ##It can be already added(2nd order of a prod in the same receipt)
                mydix[rec_t[i].casefold()].append(counter)  #insert the new index in the key list of a product in dix
        except:
            #make a new list for the specific product
            #each entry in the list is a index to a tuple(afm,product1...) in mylist
            mydix[rec_t[i].casefold()] = [counter]


def split_and_tuple(file, final_list, mydix):
    f = open(file,encoding='utf-8')
    receipt=[]
    for line in f:          ## For loop on each line of the file
        if(line[0] == "-"): # continue to the next usefull line(New receipt)
            if(len(receipt) > 0):    #Check if the LAST temp_list has usefull data(it doesnt in case of the 1st receipt)
                ####### VALIDATE INPUT #########
                res = validate_input(receipt)   #SEND ALL THE DATA OF THE LAST RECEIPT(in contrast with the dix where we send fewer data)
                ################################
                if (not(res)): #if is fault
                    receipt = [] ##Reject the last receipt
                    continue  #Continue with saving the next receipt line by line until you encounter "-"

                receipt_t = tuple(receipt[1:len(receipt)-2]) # Tuple of a receipt is ready(Without the AFM word,SUM word,and SUM)

                #####
                #Check Dictionary,add the restaurant(afm) and products so they map to the final_list index ,
                # if exists you will get the number from dix in order to recreate the tuple
                try:
                    index = mydix[receipt_t[0]]        #get index in the main list of the afm
                    temp_list = list(final_list[index])#make a list so we can calc the new data

                    for i in range(1, len(receipt_t), 4): #
                        found = False
                        for j in range(1, len(temp_list), 4):
                            if (receipt_t[i].casefold() == temp_list[j].casefold()):
                                found = True
                                temp_list[j + 1] = int(receipt_t[i + 1]) + int(temp_list[j + 1])#add the quantities and update
                                temp_list[j + 3] = float(temp_list[j + 3]) + float(receipt_t[i + 3])  #update the new sum of the product

                        if(not (found)):
                            temp_list.extend(receipt_t[i:(i+4)])
                    final_list[index] = tuple(temp_list)      # Shape the entry of the main list
                    add_to_dictionary(receipt_t, mydix, index)#update dictionary
                except:
                    add_to_dictionary(receipt_t, mydix, len(final_list))
                    temp_list = [receipt_t[0]]
                    for i in range(1, len(receipt_t), 4):        #for each product in tuple
                        quan_sum = 0 #for each product
                        sum = 0
                        prod = receipt_t[i]
                        for j in range(i, len(receipt_t), 4):      #find if there are more than 1 products like that and make the sums
                            if(prod in receipt_t[:i]):             #check if you have already calc before this product(so its already calculated because of the 1st appear of this prod.)
                                break
                            if (receipt_t[i].casefold() == receipt_t[j].casefold()):
                                quan_sum = quan_sum + int(receipt_t[j+1])
                                sum = sum + float(receipt_t[j+3])
                        if( not (sum == 0)):
                            temp_list.extend([receipt_t[i],quan_sum,receipt_t[i+2],sum]) #New product for this afm
                    final_list.append(tuple(temp_list))             ##Added to the final list of tuples

            receipt=[]      #New temp_list(for    our new receipt)
            continue        ## No other words in the line
        for word in line.split(): ##Splitting each Line into words #   Tip:  .SPLIT won't split 'Product:'
            receipt.append(word)
    f.close()
    return final_list,mydix
##########################################################################################################

##################################   2   #################################################################
def search_product(prod, mydix, mylist):
    final_list=[]
    try:
        index_list = mydix[(prod).casefold()]  # we get all the indexes where prod. appears in our list
    except:
        return             # No result
    for i in index_list:    # for each position in list
        sum = 0
        tupl = mylist[i]    #save the specific tuple entry(one of the index_list)
        #afm=tupl[0]
        for j in range(1, len(tupl), 4):  #for each entry in tuples
            if(tupl[j].casefold() == prod.casefold()): ##Compare each PRODUCT entrie in tuples
                sum = sum + float(tupl[j+3])   # Sum of the product price per AFM
        final_list.append((tupl[0].upper(), round(sum,2)))       #Build the requested list
    final_list.sort(key=lambda tup: tup[0])     #Sorting according to the 1st element of the tuples in the list
    for i in final_list:   #### Printing
        print(i[0], i[1])
##########################################################################################################

##################################   3   #################################################################
def search_afm(afm, mydix, mylist):
    final_list = []
    try:
        index = mydix[afm]
    except:
        return
    tupl = mylist[index] #All the data we got for For the specific afm
    for i in range(1, len(tupl), 4):        #for each product in tuple
        sum = 0 #for each product
        prod = tupl[i]
        for j in range(i, len(tupl), 4):      #find if there are more than 1 products like that and make a sum
            if(prod in tupl[:i]):             #check if you have already calc before this product(so its already calculated because of the 1st appear of this prod.)
                break
            if (tupl[i].casefold() == tupl[j].casefold()):
                sum = sum + float(tupl[j+3])
        if(not(sum == 0)):  ## when sum == 0 --> we are talking about an element that is already counted,and we skip it
            final_list.append((prod.upper(), round(sum, 2)))      ##add the tuple to the requested list
    final_list.sort(key=lambda tup: tup[0])
    for i in final_list:   #### Printing
        print(i[0].replace(":", ""), i[1])

##########################################################################################################
my_input=-1
mydix={}
mylist=[]
while(my_input != 4):
    my_input = input("Give your preference: (1: read new input file, 2: print statistics for a specific product, 3: print statistics for a specific AFM, 4: exit the program)")
    ########### Exception Handling ############
    #not yet handled
    try:
        my_input = int(my_input)
    except:
        my_input = -1
    ###########################################

    if(my_input == 1):
        myfile = input("Give the name of the file:")
        mylist,mydix=split_and_tuple(myfile, mylist, mydix) #a list of tuples is returned,
        # format of each touple:
        # (afm_num,prod_name1,quantity1,price_per_prod1,sum_of_prod1,prod_nameN,quantityN,price_per_prodN,sum_of_prodN,SUM)
    elif(my_input == 2):  ## Statistics according to a product
        prod = input("Insert product name:")
        search_product(prod+":", mydix, mylist)
    elif(my_input == 3): ## Statistics according to AFM
        afm = input("Insert AFM:")
        search_afm(afm, mydix, mylist)
    elif(my_input == 4):
        exit(0)
