#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "gen_code.h"
#include "literal_table.h"
#include "code_seq.h"
#include "code.h"
#include "bof.h"
#include "ast.h"
#include "utilities.h"

#define STACK_SPACE 4096

// === Initialization ===
void gen_code_initialize() {
    literal_table_initialize();
}

// === Program-Level Code Generation ===
void gen_code_program(BOFFILE bf, program_t *prog) {
    code_seq main_code = gen_code_block(prog->main_block);

    // Add exit instruction at the end
    main_code = code_seq_add_to_end(main_code, code_exit());

    // Generate and write BOF file
    BOFHeader header = {
        .magic = "BO32",
        .text_start_address = 0,
        .text_length = code_seq_size(main_code),
        .data_start_address = code_seq_size(main_code),
        .data_length = literal_table_size() * BYTES_PER_WORD,
        .stack_bottom_addr = code_seq_size(main_code) + literal_table_size() + STACK_SPACE
    };
    bof_write_header(bf, &header);
    gen_code_output_seq(bf, main_code);
    gen_code_output_literals(bf);
    bof_close(bf);
}

// Output the code sequence to the BOFFILE
static void gen_code_output_seq(BOFFILE bf, code_seq cs) {
    while (!code_seq_is_empty(cs)) {
        bof_write_word(bf, code_seq_first(cs));
        cs = code_seq_rest(cs);
    }
}

// Output literals stored in the literal table
static void gen_code_output_literals(BOFFILE bf) {
    literal_table_start_iteration();
    while (literal_table_iteration_has_next()) {
        bof_write_word(bf, literal_table_iteration_next());
    }
    literal_table_end_iteration();
}

// === Block Handling ===
static code_seq gen_code_block(block_t *block) {
    code_seq block_code = code_seq_empty();

    // Allocate space for local variables
    if (block->local_var_count > 0) {
        block_code = code_seq_add_to_end(block_code, code_addi(SP, SP, -block->local_var_count));
    }

    // Generate code for each statement in the block
    stmt_t *stmt = block->statements;
    while (stmt) {
        block_code = code_seq_concat(block_code, gen_code_stmt(stmt));
        stmt = stmt->next;
    }

    // Deallocate local variables
    if (block->local_var_count > 0) {
        block_code = code_seq_add_to_end(block_code, code_addi(SP, SP, block->local_var_count));
    }

    return block_code;
}

// === Statement Handling ===
static code_seq gen_code_stmt(stmt_t *stmt) {
    switch (stmt->kind) {
        case ASSIGN_STMT:
            return gen_code_assign_stmt(stmt->assign_stmt);
        case PRINT_STMT:
            return gen_code_print_stmt(stmt->print_stmt);
        case IF_STMT:
            return gen_code_if_stmt(stmt->if_stmt);
        case WHILE_STMT:
            return gen_code_while_stmt(stmt->while_stmt);
        case READ_STMT:
            return gen_code_read_stmt(stmt->read_stmt);
        case CALL_STMT:
            return gen_code_call_stmt(stmt->call_stmt);
        case BLOCK_STMT:
            return gen_code_block(stmt->block_stmt);
        default:
            fprintf(stderr, "Unknown statement kind.\n");
            exit(EXIT_FAILURE);
    }
}

// Assignment statement: x := expr
static code_seq gen_code_assign_stmt(assign_stmt_t assign) {
    code_seq expr_code = gen_code_expr(assign.expr);
    return code_seq_add_to_end(expr_code, code_store(assign.var.offset));
}

// Print statement: print expr
static code_seq gen_code_print_stmt(print_stmt_t print) {
    code_seq expr_code = gen_code_expr(print.expr);
    return code_seq_concat(expr_code, code_seq_singleton(code_pint()));
}

// If-Else statement
static code_seq gen_code_if_stmt(if_stmt_t if_stmt) {
    code_seq cond_code = gen_code_expr(if_stmt.cond);
    code_seq then_code = gen_code_block(if_stmt.then_branch);
    code_seq else_code = if_stmt.else_branch ? gen_code_block(if_stmt.else_branch) : code_seq_empty();

    unsigned int skip_then = code_seq_size(then_code) + 1;
    unsigned int skip_else = code_seq_size(else_code);

    code_seq if_code = cond_code;
    if_code = code_seq_add_to_end(if_code, code_branch_on_false(skip_then));
    if_code = code_seq_concat(if_code, then_code);

    if (!code_seq_is_empty(else_code)) {
        if_code = code_seq_add_to_end(if_code, code_jump(skip_else));
        if_code = code_seq_concat(if_code, else_code);
    }

    return if_code;
}

// While statement
static code_seq gen_code_while_stmt(while_stmt_t while_stmt) {
    code_seq cond_code = gen_code_expr(while_stmt.cond);
    code_seq body_code = gen_code_block(while_stmt.body);

    unsigned int body_len = code_seq_size(body_code) + 1;

    code_seq loop_code = cond_code;
    loop_code = code_seq_add_to_end(loop_code, code_branch_on_false(body_len));
    loop_code = code_seq_concat(loop_code, body_code);

    unsigned int jump_back = -(code_seq_size(loop_code) + 1);
    return code_seq_add_to_end(loop_code, code_jump(jump_back));
}

// Read statement: read x
static code_seq gen_code_read_stmt(read_stmt_t read_stmt) {
    return code_seq_singleton(code_read(read_stmt.var.offset));
}

// Procedure call: call proc
static code_seq gen_code_call_stmt(call_stmt_t call_stmt) {
    return code_seq_singleton(code_call(call_stmt.proc.offset));
}

// === Expression Handling ===
static code_seq gen_code_expr(expr_t expr) {
    switch (expr.kind) {
        case CONST_EXPR:
            return gen_code_literal(expr.literal);
        case VAR_EXPR:
            return code_seq_singleton(code_load(expr.var.offset));
        case BIN_OP_EXPR:
            return gen_code_bin_op(expr.bin_op);
        case UNARY_OP_EXPR:
            return gen_code_unary_op(expr.unary_op);
        default:
            fprintf(stderr, "Unknown expression kind.\n");
            exit(EXIT_FAILURE);
    }
}

// Generate code for a binary operation
static code_seq gen_code_bin_op(bin_op_expr_t bin_op) {
    code_seq left_code = gen_code_expr(bin_op.left);
    code_seq right_code = gen_code_expr(bin_op.right);
    code_seq op_code = code_seq_empty();

    switch (bin_op.op) {
        case ADD_OP:
            op_code = code_seq_singleton(code_add());
            break;
        case SUB_OP:
            op_code = code_seq_singleton(code_sub());
            break;
        case MUL_OP:
            op_code = code_seq_singleton(code_mul());
            break;
        case DIV_OP:
            op_code = code_seq_singleton(code_div());
            break;
        case MOD_OP:
            op_code = code_seq_singleton(code_mod());
            break;
        default:
            bail_with_error("Unsupported binary operator.");
    }

    return code_seq_concat(code_seq_concat(left_code, right_code), op_code);
}

// Generate code for a unary operation
static code_seq gen_code_unary_op(unary_op_expr_t unary_op) {
    code_seq expr_code = gen_code_expr(unary_op.expr);

    if (unary_op.op == NEG_OP) {
        return code_seq_concat(expr_code, code_seq_singleton(code_neg()));
    }

    return expr_code;
}

// Generate code for a literal
static code_seq gen_code_literal(literal_t literal) {
    unsigned int offset = literal_table_lookup(literal.value);
    return code_seq_singleton(code_load_global(offset));
}
