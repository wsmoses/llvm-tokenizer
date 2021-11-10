
    .def("getTok", [=](std::shared_ptr<llvm::LLLexer> lm) {
        const char* str1 = lm->getLoc().getPointer();
        llvm::lltok::Kind toktype = lm->Lex(); 
        const char* str2 = lm->getLoc().getPointer();
        std::string tok(str1,str2-str1);
        return py::make_tuple(to