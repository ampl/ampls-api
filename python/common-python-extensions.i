%extend ampls::AMPLModel{

   std::map<std::string, double> _getSolutionDict() {
    std::vector<double> sol = self->getSolutionVector();
    std::map<int, std::string> map = self->getVarMapInverse();
    std::map<std::string, double> res;
    std::map<int, std::string>::const_iterator it;
    for (it = map.begin(); it != map.end(); it++)
      res[it->second] = sol[it->first];
    return res;
  }
}

%extend ampls::impl::BaseCallback{

   std::map<std::string, double> _getSolutionDict() {
    std::vector<double> sol = self->getSolutionVector();
    std::map<int, std::string> map = self->getVarMapInverse();
    std::map<std::string, double> res;
    std::map<int, std::string>::const_iterator it;
    for (it = map.begin(); it != map.end(); it++)
      res[it->second] = sol[it->first];
    return res;
  }
}