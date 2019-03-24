
#Messages always have the format
#<Command | Response> <Optional Data> \n



#All supported commands:
# 'GET' optain a key-value-pair
# 'CHANGE' change a key-value-pair
# 'HELLO' ping the client


#All supported responses:
# 'GET_OK' return a key-value-pair
# 'CHANGE_OK' changed a key-value-pair


def ParseMessage(message):
    #split the string at the first blank.
    cmd,_,data=message.partition(' ')
    #print("Cmd:",cmd,"blank:",blank,"data:",data)
    return cmd,data

def ParseMessageData(data):
    key,value=data.split("=")
    return key,value


def BuildMessageCHANGE(key,value):
    return "CHANGE "+key+"="+value


#me is either ground or air (G or A)
def BuildMessageCHANGE_OK(me,key,value):
    return ("CHANGE_OK_"+me+" "+key+"="+value)


#def BuildMessageCHANGE_FAILED(key,value):
#    return ("CHANGE_FAILED "+key+"="+value)


def BuildMessageGET(key):
    return "GET "+key


#me is either ground or air (G or A)
def BuildMessageGET_OK(me,key,value):
    return ("GET_OK_"+me+" "+key+"="+value)



def BuildMessageERROR():
    return "ERROR"


def BuildMessageHELLO():
    return "HELLO"