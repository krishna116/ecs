#include <iostream>
#include <cassert>

struct Position{
  int x;
  int y;
};

struct HealthPoint{
  int value;
};

#define ECS_DEBUG
#include "data/ecs.h"  // Code generation command: ecs.exe -s "struct Position; struct HealthPoint" >> ecs.h

void test1(const char* testname) {
  printf("%s", testname);

  ecs::registry_t registry; // create registry.

  ecs::entity_t null;
  assert(!registry.isValid(null));

  auto entity = registry.create(); // create entity.
  assert(registry.isValid(entity));

  {
    auto &pos1 = registry.emplace<Position>(entity, 1, 2); // add component.
    assert(pos1.x == 1 && pos1.y == 2);

    assert(registry.has<Position>(entity));

    auto &pos2 = registry.get<Position>(entity); // get component.
    assert(pos1.x == pos2.x && pos1.y == pos2.y);

    auto &pos3 = registry.patch<Position>(entity, 5, 6); // patch component.
    assert(pos3.x == 5 && pos3.y == 6);
  }

  {
    assert(!registry.has<HealthPoint>(entity));
    auto result1 = registry.has<Position, HealthPoint>(entity);
    assert(!result1);
    registry.emplace<HealthPoint>(entity, 100);
    assert(registry.has<HealthPoint>(entity));
    auto result2 = registry.has<Position, HealthPoint>(entity);
    assert(result2);
  }

  registry.destroy(entity);
  assert(!registry.isValid(entity));
  assert(!registry.has<Position>(entity));
  assert(!registry.has<HealthPoint>(entity));
}

void test2(const char* testname){
  printf("%s", testname);
  ecs::registry_t registry;
  const int size = 5;
  for(int i = 0; i< size; ++i){
    auto entity = registry.create();
    registry.emplace<Position>(entity, 1, i);
  }

  int j = 0;
  for(auto& entity : registry){
    assert(registry.isValid(entity));
    assert(registry.has<Position>(entity));
    auto& pos = registry.get<Position>(entity);
    ++j;
  }
  assert(j == size);
}

class MyVisitor:public ecs::EntityTableVisitor{
public:
  virtual void visitEntityTableBegin() override{
    printf("visitEntityTableBegin\n");
  };

  virtual void visitEntityBegin(uint32_t entityId)override {
    printf("  visitEntityBegin id = %d\n", entityId);
  };

  virtual void visitComponent(const Position& position)override {
    printf("    {x = %d, y = %d}\n", position.x, position.y);
  };

  virtual void visitComponent(const HealthPoint& healthPoint)override {
    printf("    {hp = %d}\n", healthPoint.value);
  };

  virtual void visitEntityEnd(uint32_t entityId)override {
    printf("  visitEntityEnd id = %d\n\n", entityId);
  };

  virtual void visitEntityTableEnd()override{
    printf("visitEntityTableEnd\n");
  };
};

void test3(const char* testname){
  printf("%s", testname);

  ecs::registry_t registry;
  const int size = 5;
  for(int i = 0; i< size; ++i){
    auto entity = registry.create();
    registry.emplace<Position>(entity, 1, i);
    registry.emplace<HealthPoint>(entity, i * 10);
  }
  MyVisitor myVisitor;
  registry.visit(&myVisitor);
}

int main() {
  test1("[Test common api]\n");
  test2("[Test iteration]\n");
  test3("[Test visitor]\n");

  return 0;
}
