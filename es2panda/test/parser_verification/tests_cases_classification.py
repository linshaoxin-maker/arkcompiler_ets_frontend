# -*- coding: utf-8 -*-
import subprocess
import time
import sys
import os

ret_ts=[]
ret_error=[]
AllFilesNum=0
AllTsFilesNum=0
ErrorFilesNum=0
def DecideRoot(rootpath):
    fileList = os.listdir(rootpath)
    for folderName in fileList:
        path = os.path.join(rootpath,folderName)
        if(folderName=="compiler" or folderName=="conformance"):
            CountTsFiles(path)
        else:
            continue
        
## to Count the numbe of .ts file in the target rootpath
def CountTsFiles(rootpath):
    global AllFilesNum
    global AllTsFilesNum
    fileList = os.listdir(rootpath)
    for fileName in fileList:
        path = os.path.join(rootpath,fileName)
        if(os.path.isfile(path)):
            AllFilesNum+=1
            if(path.endswith(".ts")):
                ret_ts.append(path)
                AllTsFilesNum += 1
        else:
            CountTsFiles(path)

    
            
def writeFileNameIO(path,lst):
    writeIO=open(path,"w")
    for file in lst:
        writeIO.write(file+"\n")
    writeIO.close()
    
            
def CountTsErrorFiles(baselineRootPath,rootSaveDir):
    global ErrorFilesNum
    fileList = os.listdir(baselineRootPath);
    for fileName in fileList:
        path = os.path.join(baselineRootPath,fileName)
        if(os.path.isfile(path)):
            if(path.endswith("errors.txt")):
                ErrorFilesNum += 1
                ret_error.append(path)         
        #else:
            #CountErrorFiles(path);

    

## get the right ts file path
tsRightFileList=[]
tsCompilerConformanceErrorFileList=[]
def getRightTsFile(allTsfile,errorTsfile):
    errorTsFileList=[]
    ReadIO=open(errorTsfile,"r")
    for line in ReadIO:
        tsName=line.strip().split("/")[-1].replace(".errors.txt","")+".ts"
        errorTsFileList.append(tsName)
    ReadIO.close()
    errorTsFileSet = set(errorTsFileList)
    
    print("The number of errorNameList: "+str(len(errorTsFileList)))  #The number of errorNameList: 4977
    print("The number of errorNameSet: "+str(len(errorTsFileSet)))    #The number of errorNameSet: 4977
    
    ReadIO2=open(allTsfile,"r")
    tsAllFileList=[]
    for line in ReadIO2:
        tsName2=line.split("/")[-1].strip()
        tsAllFileList.append(tsName2)
        if(tsName2 not in errorTsFileSet):
            tsRightFileList.append(line.strip())
        else:
            tsCompilerConformanceErrorFileList.append(line.strip())
    ReadIO2.close()

    
    #from collections import Counter
    #dict1 = Counter(tsAllFileList)
    # for k,v in dict1.items():
    #     if(v>1):
    #         print("k:+\n")
    #         print(k)    -->get two exceptions
    #tsAllFileSet = set(tsAllFileList)
    #print("The number of tsFileNameList: "+str(len(tsAllFileList)))  #The number of tsFileNameList: 10107
    #print("The number of tsFileNameSet: "+str(len(tsAllFileSet)))   #The number of tsFileNameSet: 10106
    #inter=tsAllFileSet.intersection(errorTsFileSet)
    #print("the number of intersection: "+str(len(inter)))  #the number of intersection: 4753
    #print("The number of right ts file: "+str(len(tsRightFileList))) 

# getRightTsFile(tsAllFilePath,tscerrorFilePath)   

# tscCompilerConformanceErrorFilePath="/mnt/disk4/zhangchen/TypeScript/chen/StatiscTestCases/tscCompilerConformanceErrorFileList.txt"
# writeFileNameIO( tscCompilerConformanceErrorFilePath, tsCompilerConformanceErrorFileList)

def executeCommand(filePath,rootSaveDir,es2abcPath):
    fileIO=open(filePath,"r")
    suncessNum=0
    failNum=0
    es2pandaFailList=[]
    es2pandaSuccessList=[]
    '''
    exception = "third_party/typescript/tests/cases/conformance/classes/propertyMemberDeclarations/abstractProperty.ts"
    exception2 = "third_party/typescript/tests/cases/compiler/abstractProperty.ts"
    '''
    for line in fileIO:
        #filePath="/mnt/disk4/zhangchen/ohos/third_party/typescript/tests/cases/compiler/abstractClassInLocalScope.ts"
        filePath=line.strip()
        ## es2panda AST does not write to file
        ''' 
        if(filePath.find(exception)>=0):
            fileName="abstractProperty_comformance.json"
        elif(filePath.find(exception)>=0):
            fileName="abstractProperty_compiler.json"
        else:
            fileName = line.strip().split("/")[-1].replace(".ts","")+".json"
        outRootSuccessPath="/mnt/disk4/zhangchen/TypeScript/chen/es2panda_ast/DumpSuccessFolder"
        outRootFailPath="/mnt/disk4/zhangchen/TypeScript/chen/es2panda_ast/DumpFailFolder"
        outSuccessPath= os.path.join(outRootSuccessPath,fileName)
        outFailPath= os.path.join(outRootFailPath,fileName)
        '''
        half=filePath # +" > "+outSuccessPath     ## es2panda AST does not write to file
        ## get the ralative path
        #command = "/mnt/disk4/zhangchen/ohos/out/rk3568/clang_x64/exe.unstripped/clang_x64/ark/ark/es2abc --extension ts --dump-ast --parse-only " + half
        command= es2abcPath+ " --extension ts --dump-ast --parse-only --module " + half
        subp = subprocess.Popen(command,shell=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
        out, err = subp.communicate()
        if subp.returncode == 0:
            #out, err = subp.communicate()
            suncessNum += 1
            es2pandaSuccessList.append(line)
        else:
            #out, err = subp.communicate()
            failNum += 1
            '''
            WriteIO=open(outFailPath.replace(".json",".txt"),"w+")
            WriteIO.write(err)
            WriteIO.close()
            '''
            es2pandaFailList.append(line)
            
    fileIO.close()
    print("es2panda Success Num: "+str(suncessNum))
    print("es2panda Fail Num: "+str(failNum))
    
    ## save the fail file
    es2pandaFailFilePath= os.path.join(rootSaveDir,"es2panFailFileList.txt")        
    es2pandaFailIO=open(es2pandaFailFilePath,"w")
    for line in es2pandaFailList:
          es2pandaFailIO.write(line)
    es2pandaFailIO.close()
    ## save the success file
    es2pandaSuccessFilePath= os.path.join(rootSaveDir,"es2panSuccessFileList.txt")        
    es2pandaSuccessIO=open(es2pandaSuccessFilePath,"w")
    for line in es2pandaSuccessList:
          es2pandaSuccessIO.write(line)
    es2pandaSuccessIO.close()

#param1: tscTestsRootPath --> The root path of tsc tests                    eg:"/mnt/disk4/zhangchen/ohos/third_party/typescript/tests"
#param2: ResultSaveDir    --> Save the statistics files in this directory   eg:"/mnt/disk4/zhangchen/TypeScript/chen/es2panda_ast/saveDir"


def main():
    if(len(sys.argv) <2):
        sys.exit(" Parameters are not enough")
    time_start = time.time()    
    tscTestsRootPath=sys.argv[1]   #"eg:"/mnt/disk4/zhangchen/ohos/third_party/typescript/tests"
    ResultSaveDir=sys.argv[2]       # eg:"/mnt/disk4/zhangchen/TypeScript/chen/es2panda_ast/saveDir"
    print(tscTestsRootPath)
    print(ResultSaveDir)
    ## create the dir
    if not os.path.exists(ResultSaveDir):
        os.makedirs(ResultSaveDir)
    ## Get all the paths    
    tsRootPathDir=os.path.join(tscTestsRootPath,"cases")   #eg:"/mnt/disk4/zhangchen/ohos/third_party/typescript/tests/cases/  
    baselineRootPath=os.path.join(tscTestsRootPath,"baselines/reference")    #/mnt/disk4/zhangchen/ohos/third_party/typescript/tests/baselines/reference
    ohosRootPath = tscTestsRootPath.replace("/third_party/typescript/tests","")  #/mnt/disk4/zhangchen/ohos
    es2abcPath=os.path.join(ohosRootPath,"out/rk3568/clang_x64/exe.unstripped/clang_x64/ark/ark/es2abc") #eg:/mnt/disk4/zhangchen/ohos/out/rk3568/clang_x64/exe.unstripped/clang_x64/ark/ark/es2abc
    ## Execute all the functions
    DecideRoot(tsRootPathDir)
    print("The total number of files: "+str(AllFilesNum))
    print("The number of .ts files: "+str(AllTsFilesNum))
    tsAllFilePath=os.path.join(ResultSaveDir,"tscTestFileList.txt")        
    writeFileNameIO(tsAllFilePath,ret_ts)
    
    CountTsErrorFiles(baselineRootPath,ResultSaveDir)        
    tsErrorFilePath=os.path.join(ResultSaveDir,"tscErrorFileList.txt")        
    writeFileNameIO(tsErrorFilePath,ret_error)
    
    getRightTsFile(tsAllFilePath,tsErrorFilePath)
    tscRightFilePath=os.path.join(ResultSaveDir,"tscRightFileList.txt")  
    writeFileNameIO(tscRightFilePath, tsRightFileList)
    
    executeCommand(tscRightFilePath,ResultSaveDir,es2abcPath)
    
    ## compute the cost time
    time_end = time.time()
    time_sum = time_end - time_start
    print("The execitopm time of program(s): "+ str(time_sum))
    print("All finihsed")
    
if __name__ == "__main__":
    main()

## python chen/StatiscTestCases/demo3.py  /mnt/disk4/zhangchen/ohos/third_party/typescript/tests  /mnt/disk4/zhangchen/TypeScript/chen/es2panda_ast/saveModuleDir