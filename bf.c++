#include<queue>
#include <vector>
#include <iostream>

using namespace std;
void bfs(int src,vector<vector<int>>& adjl){
    queue<int> q;
    q.push(src);
    int number_of_nodes = adjl.size();
    vector<bool> visited(number_of_nodes,false);
    visited[src] = true;

    while(!q.empty()){         
        int v = q.front();
        q.pop();

        for(auto& x : adjl[v]){
            if(!visited[x]){
                visited[x] = true;
                q.push(x);
            }
        }

    }
 }