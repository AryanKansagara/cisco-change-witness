#include <algorithm>
#include <functional>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <string>
#include <vector>
struct Edge {int from,to,enabled;};
struct Change {int edge,enabled;};
struct Model {int n,s,t;std::vector<Edge> edges;std::vector<Change> changes;};
Model read(std::istream& in) {
    int n,m,k,s,t;
    if(!(in>>n>>m>>k>>s>>t)||n<2||n>8||m<0||m>32||k<0||k>12||s<0||s>=n||t<0||t>=n||s==t)
        throw std::runtime_error("invalid header: N M K source target; bounds N2..8 M0..32 K0..12");
    Model model{n,s,t,{}, {}};
    for(int i=0;i<m;++i){int a,b,on;if(!(in>>a>>b>>on)||a<0||a>=n||b<0||b>=n||(on!=0&&on!=1))throw std::runtime_error("invalid edge");model.edges.push_back({a,b,on});}
    std::vector<bool> used(static_cast<std::size_t>(m));
    for(int i=0;i<k;++i){int e,on;if(!(in>>e>>on)||e<0||e>=m||(on!=0&&on!=1)||used.at(static_cast<std::size_t>(e)))throw std::runtime_error("invalid change or repeated edge target");used.at(static_cast<std::size_t>(e))=true;model.changes.push_back({e,on});}
    std::string extra;if(in>>extra)throw std::runtime_error("unexpected trailing input");return model;
}
std::vector<int> reachable(const Model& model,int mask){
    auto edges=model.edges;
    for(std::size_t j=0;j<model.changes.size();++j)if(mask&(1<<j))edges.at(static_cast<std::size_t>(model.changes[j].edge)).enabled=model.changes[j].enabled;
    std::vector<int> seen(static_cast<std::size_t>(model.n));std::queue<int> q;q.push(model.s);seen.at(static_cast<std::size_t>(model.s))=1;
    while(!q.empty()){int at=q.front();q.pop();for(auto e:edges)if(e.enabled&&e.from==at&&!seen.at(static_cast<std::size_t>(e.to))){seen.at(static_cast<std::size_t>(e.to))=1;q.push(e.to);}}
    std::vector<int> result;for(int i=0;i<model.n;++i)if(seen.at(static_cast<std::size_t>(i)))result.push_back(i);return result;
}
int bits(int mask){int count=0;while(mask){count+=mask&1;mask>>=1;}return count;}
void array(const std::vector<int>& values){std::cout<<'[';for(std::size_t i=0;i<values.size();++i){if(i)std::cout<<',';std::cout<<values[i];}std::cout<<']';}
void solve(const Model& model){
    int count=1<<model.changes.size(),full=count-1;
    std::vector<int> safe(static_cast<std::size_t>(count)),bad;
    for(int mask=0;mask<count;++mask){auto r=reachable(model,mask);safe.at(static_cast<std::size_t>(mask))=std::find(r.begin(),r.end(),model.t)!=r.end();if(!safe.at(static_cast<std::size_t>(mask)))bad.push_back(mask);}
    int minimal=-1;for(int mask:bad)if(minimal<0||bits(mask)<bits(minimal))minimal=mask;
    std::vector<int> failed(static_cast<std::size_t>(count)),order;
    std::function<bool(int)> search=[&](int mask){
        if(!safe.at(static_cast<std::size_t>(mask))||failed.at(static_cast<std::size_t>(mask)))return false;
        if(mask==full)return true;
        for(std::size_t j=0;j<model.changes.size();++j)if(!(mask&(1<<j))){order.push_back(static_cast<int>(j));if(search(mask|(1<<j)))return true;order.pop_back();}
        failed.at(static_cast<std::size_t>(mask))=1;return false;
    };
    bool found=search(0);
    std::cout<<"{\"schema_version\":1,\"model\":\"directed-allow-edge-v1\",\"states_examined\":"<<count<<",\"baseline_safe\":"<<(safe[0]?"true":"false")<<",\"final_safe\":"<<(safe.at(static_cast<std::size_t>(full))?"true":"false")<<",\"unsafe_masks\":";array(bad);
    std::cout<<",\"safe_order\":";if(found)array(order);else std::cout<<"null";
    std::cout<<",\"counterexample\":";
    if(minimal<0)std::cout<<"null";else{
        std::vector<int> subset;for(std::size_t j=0;j<model.changes.size();++j)if(minimal&(1<<j))subset.push_back(static_cast<int>(j));
        std::cout<<"{\"source\":"<<model.s<<",\"target\":"<<model.t<<",\"mask\":"<<minimal<<",\"minimum_cardinality_changes\":";array(subset);std::cout<<",\"reachable_cut\":";array(reachable(model,minimal));std::cout<<'}';
    }
    std::cout<<"}\n";
}
int main(int argc,char** argv){
    try{
        if(argc==2&&std::string(argv[1])=="--demo"){
            // Migrate from 0->1->3 to 0->2->3. Deletions before additions fail.
            Model m{4,0,3,{{0,1,1},{1,3,1},{0,2,0},{2,3,0}},{{0,0},{1,0},{2,1},{3,1}}};solve(m);
        }else if(argc==1)solve(read(std::cin));else throw std::runtime_error("use stdin or --demo");
    }catch(const std::exception&){std::cerr<<"{\"error\":\"invalid restricted graph input\"}\n";return 2;}
}
