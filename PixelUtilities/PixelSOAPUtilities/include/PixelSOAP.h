struct Attribute { std::string name_; std::string value_; };
typedef std::vector<Attribute> Attribute_Vector;

struct Variable { std::string name_; std::string type_; std::string payload_;};
typedef std::vector<Variable> Variable_Vector;

struct PixelSOAPCommand { std::string command_; Attribute_Vector parameters_;};
typedef std::vector<PixelSOAPCommand> PixelSOAPCommands;
