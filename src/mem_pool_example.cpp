#include "mem_pool.h"

struct Mystruct{
    int m_data[3];
};


int main()
{
    using namespace Common;
    MemPool<double> pod_pool(50);
    MemPool<Mystruct> struct_pool(50);
    for (auto i=0; i<50; ++i) {
        auto *p_ret=pod_pool.allocate(i);
        auto *s_ret= struct_pool.allocate(Mystruct{i,i+1,i+2});
        std::cout<<"pod element:"<<*p_ret<<" allocted at:"<<
            p_ret<<std::endl;
        std::cout<<"strcuct element:"<<s_ret->m_data[0]<<" , "<<
            s_ret->m_data[1]<<" , "<<s_ret->m_data[2]<<" allocted at:"<<
                s_ret<<std::endl;
        if (i%5==0) {
            std::cout<<"deallocating pod element: "<<*p_ret<<" from: "
            <<p_ret<<std::endl;
            std::cout<<"deallocating struct element:"<<s_ret->m_data[0]<<" , "<<
            s_ret->m_data[1]<<" , "<<s_ret->m_data[2]<<" from "
            <<s_ret<<std::endl;

            pod_pool.deallocate(p_ret);
            struct_pool.deallocate(s_ret);

        
        }
    }

    return 0;
}