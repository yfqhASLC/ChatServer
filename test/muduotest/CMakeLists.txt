cmake_minimum_required(VERSION 3.0)#需要的最低cmake版本     这里是>=3.0
project(main)#项目名称mian



#配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)#-g表示使生成的可执行文件可以调试


#指定搜索的子目录
#add_subdirectory()


#配置头文件搜索路径
#include_directories()


#配置库文件搜索路径
#link_directories()



#设置需要编译的源文件列表
set(SRC_LIST ./src/muduo_server.cpp)#./表示当前目录下


#设置生成的可执行文件的存储路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)#PROJECT_SOURCE_DIR 工程的根目录


#aux_source_directory(. ${SRC_LSIT})#把 . 指定路径下的所有源文件名字放入变量名SRC_LIST中


#表示生成的可执行文件server，是由SRC_LIST变量所定义的源文件编译而来
add_executable(server ${SRC_LIST})
#源文件很少时，可以直接：add_executable(server muduo_server.cpp)


#表示server这个目标程序需要链接 muduo_net muduo_base pthread 这三个库文件
target_link_libraries(server muduo_net muduo_base pthread)#不用写-l