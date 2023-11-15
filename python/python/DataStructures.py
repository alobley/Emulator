class CustomStack:
    def __init__(self):
        self._stack = []
        self.length = 0
        
    def push(self, value):
        if len(self._stack) > 0:
            tempStack = []
            for i in range(len(self._stack)):
                tempStack.append(self._stack[i])
        
            self._stack = [value]
        
            for i in range(len(tempStack) + 1):
                if i != 0:
                    self._stack.append(tempStack[i-1])
        if len(self._stack) == 0:
            self._stack = [value]
        self.length += 1
                
    def pop(self):
        if len(self._stack) > 0:
            poppedVal =  self._stack[0]
            del(self._stack[0])
            self.length -= 1
            return poppedVal
        else:
            print("Error: List size is 0.")
    
    def peek(self):
        return self._stack[0]
    



class CustomSingleLinkList:
    def __init__(self):
        self.list = []
        self.Front = None
        self.Next = None
        self.Current = None
        self.length = 0
        
        
    def AddAtEnd(self, value):
        if len(self.list) == 0:
            self.Front = value
            
        self.list.append(value)
        self.length += 1
        
    def AddAtBeginning(self, value):
        if len(self.list) == 0:
            self.list.append(value)
            
        self.Front = value
        if len(self.list) > 0:
            tempStack = []
            for i in range(len(self.list)):
                tempStack.append(self.list)
        
            self.list = []
            self.list.append(value)
        
            for i in range(len(self.list) + 1):
                if i > 0:
                    self.list.append(tempStack[i-1])
        else:
            self.list.append(value)
        self.length += 1
            
    def GetList(self):
        return self.list