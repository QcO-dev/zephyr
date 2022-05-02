#include "ast.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* data_type_to_string(DataType type) {
	switch(type) {
		case DATA_TYPE_VOID: return "void";
		case DATA_TYPE_INT: return "int";
		case DATA_TYPE_I8: return "i8";
		case DATA_TYPE_I16: return "i16";
		case DATA_TYPE_I32: return "i32";
		case DATA_TYPE_I64: return "i64";
		case DATA_TYPE_ANY: return "any";
		case DATA_TYPE_UNRESOLVED:
		case DATA_TYPE_UNION:
		case DATA_TYPE_STRUCT: assert(0 && "Unreachable");
	}
	return "<unknown type>";
}

char* node_type_to_string(NodeType type) {
	switch(type) {
#define E(name, str) case name: return str;
	ENUM_AST(E)
#undef E
	}
	return "<unknown node type>";
}

char* struct_name_string(Type strukt) {
	char* str = malloc(strukt.name.length + 1);
	memcpy(str, strukt.name.start, strukt.name.length);
	str[strukt.name.length] = '\0';
	return str;
}

char* type_to_string(Type type) {
	char* str = malloc(128); // Should be enough for now
	char* baseType;

	if(type.type == DATA_TYPE_STRUCT || type.type == DATA_TYPE_UNION || type.type == DATA_TYPE_UNRESOLVED)
		baseType = struct_name_string(type);
	else
		baseType = data_type_to_string(type.type);

	strcpy(str, baseType);
	
	for(int i = 0; i < type.indirection; i++) {
		strcat(str, "*");
	}
	return str;
}

bool is_unary_op(NodeType type) {
	switch(type) {
		case OP_BWNOT:
		case OP_NEG:
		case OP_NOT:
		case OP_ADDROF:
		case OP_DEREF:
			return true;
		default:
			return false;
	}
}

bool is_binary_op(NodeType type) {
	switch(type) {
		case OP_ADD:
		case OP_SUB:
		case OP_MUL:
		case OP_DIV:
		case OP_MOD:
		case OP_BWAND:
		case OP_BWOR:
		case OP_XOR:
		case OP_LSH:
		case OP_RSH:
		case OP_EQUAL:
		case OP_NOT_EQUAL:
		case OP_LESS:
		case OP_LESS_EQ:
		case OP_GREATER:
		case OP_GREATER_EQ:
		case OP_OR:
		case OP_AND:
			return true;
		default:
			return false;
	}
}

void print_ast_depth(Node* node, int depth) {
	for(int i = 0; i < depth; i++) printf("  ");

	if(is_unary_op(node->type)) {
		printf("%s\n", node_type_to_string(node->type));
		print_ast_depth(node->unary, depth + 1);
		return;
	}

	else if(is_binary_op(node->type)) {
		printf("%s\n", node_type_to_string(node->type));
		print_ast_depth(node->binary.lhs, depth + 1);
		printf("\n");
		print_ast_depth(node->binary.rhs, depth + 1);
		return;
	}

	switch(node->type) {
		case AST_PROGRAM: {
			for(size_t i = 0; i < node->block.size; i++)
				print_ast_depth(node->block.children[i], depth);
			break;
		}
		case AST_BLOCK: {
			printf("{\n");
			for(size_t i = 0; i < node->block.size; i++)
				print_ast_depth(node->block.children[i], depth + 1);
			for(int i = 0; i < depth; i++) printf("  ");
			printf("}\n");
			break;
		}

		case AST_EXPR_STMT: {
			print_ast_depth(node->unary, 0);
			printf("\n");
			break;
		}

		case AST_CAST: {
			printf("cast\n");
			print_ast_depth(node->unary, depth);
			printf("\nas %s", type_to_string(node->computedType));
			break;
		}

		case AST_FUNCTION: {
			printf("function %.*s: %s\n", (int)node->function.name.length, node->function.name.start, type_to_string(node->function.returnType));
			for(int i = 0; i < node->function.argumentCount; i++) {
				Node* arg = node->function.arguments[i];
				printf("(%.*s: %s) ", (int)arg->variable.name.length, arg->variable.name.start, type_to_string(arg->variable.type));
			}
			if(!node->function.hasImplicitBody)
				print_ast_depth(node->function.body, depth);
			break;
		}

		case AST_IF: {
			printf("if\n");
			print_ast_depth(node->conditional.condition, depth + 1);
			printf("\n");
			print_ast_depth(node->conditional.doTrue, depth + 1);
			printf("\n");

			if(node->conditional.doFalse != NULL) {
				for(int i = 0; i < depth; i++) printf("  ");
				printf("else\n");
				print_ast_depth(node->conditional.doFalse, depth + 1);
				printf("\n");
			}
			break;
		}

		case AST_FOR: {
			printf("for\n");
			if(node->loop.initial != NULL)
				print_ast_depth(node->loop.initial, depth + 1);
			else {
				for(int i = 0; i < depth + 1; i++) printf("  ");
				printf("(no initializer)\n");
			}

			if(node->loop.condition != NULL) {
				print_ast_depth(node->loop.condition, depth + 1);
				printf("\n");
			} else {
				for(int i = 0; i < depth + 1; i++) printf("  ");
				printf("(no condition)\n");
			}

			if(node->loop.iteration != NULL) {
				print_ast_depth(node->loop.iteration, depth + 1);
				printf("\n");
			} else {
				for(int i = 0; i < depth + 1; i++) printf("  ");
				printf("(no iteration)\n");
			}

			print_ast_depth(node->loop.body, depth + 1);
			break;
		}

		case AST_WHILE: {
			printf("while\n");
			print_ast_depth(node->conditional.condition, depth + 1);
			printf("\n");
			print_ast_depth(node->conditional.doTrue, depth + 1);
			printf("\n");
			break;
		}

		case AST_RETURN: {
			printf("return\n");
			if(node->unary)
				print_ast_depth(node->unary, depth + 1);
			else {
				for(int i = 0; i < depth + 1; i++) printf("  ");
				printf("(void)");
			}
			printf("\n");
			break;
		}

		case AST_INT_LITERAL: {
			printf("(literal %ld)", node->literal.as.integer);
			break;
		}

		case AST_CHAR_LITERAL: {
			printf("(literal '%c')", (int)node->literal.as.integer);
			break;
		}

		case AST_STRING: {
			printf("(literal \"%.*s\")", (int)node->literal.as.string.length, node->literal.as.string.chars);
			break;
		}

		case AST_DEFINE_GLOBAL_VAR:
		case AST_DEFINE_VAR: {
			printf("var %.*s: %s ", (int)node->variable.name.length, node->variable.name.start, type_to_string(node->variable.type));
			if(node->variable.value != NULL) {
				print_ast_depth(node->variable.value, 0);
			}
			printf("\n");
			break;
		}

		case AST_ACCESS_GLOBAL_VAR:
		case AST_ACCESS_VAR: {
			printf("(var %.*s)", (int)node->variable.name.length, node->variable.name.start);
			break;
		}

		case AST_ASSIGN_GLOBAL_VAR:
		case AST_ASSIGN_VAR: {
			printf("(%.*s = ", (int)node->variable.name.length, node->variable.name.start);
			print_ast_depth(node->variable.value, 0);
			printf(")");
			break;
		}

		case AST_DEFINE_CONST: {
			printf("const %.*s = %ld", (int)node->constant.name.length, node->constant.name.start, node->constant.value);
			printf("\n");
			break;
		}

		case AST_CALL: {
			printf("(call %.*s", (int)node->function.name.length, node->function.name.start);
			for(int i = 0; i < node->function.argumentCount; i++) {
				Node* arg = node->function.arguments[i];
				printf(" ");
				print_ast_depth(arg, 0);
			}
			printf(")");
			break;
		}

		case AST_CALL_METHOD: {
			printf("(call %.*s", (int)node->function.name.length, node->function.name.start);
			for(int i = 0; i < node->function.argumentCount; i++) {
				Node* arg = node->function.arguments[i];
				printf(" ");
				print_ast_depth(arg, 0);
			}
			printf(") of\n");
			print_ast_depth(node->function.parent, depth + 1);
			break;
		}

		case OP_TERNARY: {
			printf("(");
			print_ast_depth(node->conditional.condition, 0);
			printf(") ?\n");
			print_ast_depth(node->conditional.doTrue, depth + 1);
			printf("\n");
			print_ast_depth(node->conditional.doFalse, depth + 1);
			break;
		}

		case OP_SIZEOF: {
			printf("sizeof(%s)", type_to_string(node->computedType));
			break;
		}

		case OP_ACCESS_SUBSCRIPT: {
			printf("subscript [\n");
			print_ast_depth(node->binary.lhs, depth + 1);
			printf("\n");
			print_ast_depth(node->binary.rhs, depth + 1);
			printf("\n");
			for(int i = 0; i < depth + 1; i++) printf("  ");
			printf("]");
			break;
		}

		case OP_ASSIGN_SUBSCRIPT: {
			printf("subscript = [\n");
			print_ast_depth(node->ternary.lhs, depth + 1);
			printf("\n");
			print_ast_depth(node->ternary.mid, depth + 1);
			printf("\n");
			print_ast_depth(node->ternary.rhs, depth + 1);
			printf("\n");
			for(int i = 0; i < depth + 1; i++) printf("  ");
			printf("]");
			break;
		}

		case AST_ARRAY_INIT: {
			printf("[\n");
			for(size_t i = 0; i < node->block.size; i++) {
				print_ast_depth(node->block.children[i], depth + 1);
				printf("\n");
			}
			for(int i = 0; i < depth + 1; i++) printf("  ");
			printf("]\n");

			break;
		}

		case OP_ASSIGN_DEREF: {
			printf("* =\n");
			print_ast_depth(node->binary.lhs, depth + 1);
			printf("\n");
			print_ast_depth(node->binary.rhs, depth + 1);
			break;
		}

		case AST_UNION: {
			printf("union %.*s\n", (int)node->variable.name.length, node->variable.name.start);
			for(int i = 0; i < node->computedType.fieldCount; i++) {
				print_ast_depth(node->computedType.fields[i], depth + 1);
			}
			break;
		}

		case AST_STRUCT: {
			printf("struct %.*s\n", (int)node->variable.name.length, node->variable.name.start);
			for(int i = 0; i < node->computedType.fieldCount; i++) {
				print_ast_depth(node->computedType.fields[i], depth + 1);
			}
			break;
		}

		case AST_MEMBER: {
			printf("%.*s: %s\n", (int)node->variable.name.length, node->variable.name.start, type_to_string(node->variable.type));
			break;
		}

		case OP_ACCESS_MEMBER: {
			printf(".%.*s\n", (int)node->member.name.length, node->member.name.start);
			print_ast_depth(node->member.parent, depth + 1);
			break;
		}

		case OP_ASSIGN_MEMBER: {
			printf(".%.*s\n", (int)node->member.name.length, node->member.name.start);
			print_ast_depth(node->member.parent, depth + 1);
			printf("\n");
			print_ast_depth(node->member.value, depth + 1);
			break;
		}

		default: {
			fprintf(stderr, "Cannot handle type '%s' in print_ast_depth\n", node_type_to_string(node->type));
			break;
		}
	}
}

void print_ast(Node* ast) {
	print_ast_depth(ast, 0);
}

Node* new_node(NodeType type, Token position) {
	Node* node = calloc(1, sizeof(Node));
	node->type = type;
	node->position = position;
	return node;
}

void node_add_child(Node* parent, Node* child) {
	if(parent->block.size + 1 > parent->block.capacity) {
		parent->block.capacity = parent->block.capacity < 8 ? 8 : parent->block.capacity * 2;
		parent->block.children = realloc(parent->block.children, parent->block.capacity * sizeof(Node*));
	}
	parent->block.children[parent->block.size++] = child;
}