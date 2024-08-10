#定义了一个SRC_LIST变量，包含了该目录下(不包含子目录下的)所有的源文件
aux_source_directory(. SRC_LIST)


#指定生成可执行文件
add_executable(ChatClient ${SRC_LIST})


#指定可执行文件链接时依赖的库文件
target_link_libraries(ChatClient pthread)