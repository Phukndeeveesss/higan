struct SystemOverview : Panel {
  SystemOverview(View*);
  auto show() -> void override;
  auto hide() -> void override;
  auto refresh() -> void;
  auto scan(higan::Node::Object node, uint depth = 1) -> void;

  ListView nodeList{this, Size{~0, ~0}};
};
