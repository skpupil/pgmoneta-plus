/*
 * Copyright (C) 2022 Red Hat
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may
 * be used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* pgmoneta */
#include <pgmoneta.h>
#include <workflow.h>

/* system */
#include <stdlib.h>

static struct workflow* wf_backup(void);

struct workflow*
pgmoneta_workflow_create(int workflow_type)
{
   switch (workflow_type)
   {
      case WORKFLOW_TYPE_BACKUP:
         return wf_backup();
         break;
      default:
         break;
   }

   return NULL;
}

int
pgmoneta_workflow_delete(struct workflow* workflow)
{
   struct workflow* wf = NULL;
   struct workflow* nxt = NULL;

   if (workflow != NULL)
   {
      wf = workflow;
      nxt = wf->next;

      while (wf != NULL)
      {
         nxt = wf->next;

         free(wf);

         wf = nxt;
      }
   }

   return 0;
}

static struct workflow*
wf_backup(void)
{
   struct workflow* head = NULL;
   struct workflow* current = NULL;
   struct configuration* config = NULL;

   config = (struct configuration*)shmem;

   head = pgmoneta_workflow_create_basebackup();
   current = head;

   if (config->compression_type == COMPRESSION_GZIP)
   {
      current->next = pgmoneta_workflow_create_gzip();
      current = current->next;
   }
   else if (config->compression_type == COMPRESSION_ZSTD)
   {
      current->next = pgmoneta_workflow_create_zstd();
      current = current->next;
   }
   else if (config->compression_type == COMPRESSION_LZ4)
   {
      current->next = pgmoneta_workflow_create_lz4();
      current = current->next;
   }

   if (config->link)
   {
      current->next = pgmoneta_workflow_create_link();
      current = current->next;
   }

   return head;
}
